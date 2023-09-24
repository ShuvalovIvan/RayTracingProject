#include <vulkan/vulkan_core.h>
#include <Windows.h>

#include "graphical_environment_vulkan.h"

#include <bitset>

#include "shader_loader.h"


namespace VulkanImpl
{

    void GraphicalEnvironment::enable_validation() {
        _validation = std::make_unique<Validation>();
    }

    void GraphicalEnvironment::init()
    {
        std::clog << "Init Vulkan" << std::endl;
#   ifdef _WIN32
        char buffer[MAX_PATH] = { 0 };
        GetModuleFileName( NULL, buffer, MAX_PATH );
        std::clog << "CWD " << buffer << std::endl;
#   endif
        if (!glfwInit())
        {
            LOG_AND_THROW(std::runtime_error("glfwInit() failed"));
        }

        if (!glfwVulkanSupported())
        {
            LOG_AND_THROW(std::runtime_error("glfwVulkanSupported() failed"));
        }

        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Vulkan Project";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "No Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_0;

        uint32_t glfwExtensionCount = 0;
        auto glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
        if (glfwExtensionCount <= 0) {
            LOG_AND_THROW(std::runtime_error("No instance extensions found"));
        }
        std::vector<const char*> extensions;
        std::copy(&glfwExtensions[0], &glfwExtensions[glfwExtensionCount], back_inserter(extensions));

        if (_validation != nullptr) {
            _validation->append_extensions(&extensions);
        }
        std::clog << "Extensions found with GLFW: " << extensions.size() << " ";
        std::copy(extensions.begin(), extensions.end(), std::ostream_iterator<const char *>(std::clog, "\n"));

        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;
        createInfo.enabledExtensionCount = extensions.size();
        createInfo.ppEnabledExtensionNames = &extensions[0];

        auto layers = _validation->supported_layers();
        createInfo.enabledLayerCount	= layers.size();
        createInfo.ppEnabledLayerNames	= layers.data();

        if (VK_SUCCESS != vkCreateInstance(&createInfo, nullptr, &_instance)) {
            LOG_AND_THROW(std::runtime_error("create instance"));
        }
        std::clog << "Instance created" << std::endl;
        if (_validation != nullptr) {
            _validation->init(_instance);
        }

        window_init();
        std::clog << "Window initialized" << std::endl;
        surface_init();
        std::clog << "Surface initialized" << std::endl;
        _device = std::make_unique<Device>();
        _device->init(_instance, _surface, _window);
        _shader_modules = std::make_unique<ShaderModules>(*_device.get());

        _descriptor_set_layout = std::make_unique<DescriptorSetLayout>(*_device.get());
        _descriptor_set_layout->init();
    }

    void GraphicalEnvironment::window_init() {
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

        _window = glfwCreateWindow(800, 600, "Vulkan project", nullptr, nullptr);
        if (_window == nullptr)
        {
            LOG_AND_THROW(std::runtime_error("failed to create window"));
        }

        glfwSetWindowUserPointer(_window, this);
        // glfwSetKeyCallback(_window, GlfwKeyCallback);
        // glfwSetCursorPosCallback(window_, GlfwCursorPositionCallback);
        // glfwSetMouseButtonCallback(window_, GlfwMouseButtonCallback);
        // glfwSetScrollCallback(window_, GlfwScrollCallback);
    }

    void GraphicalEnvironment::surface_init() {
        if (VK_SUCCESS != glfwCreateWindowSurface(_instance, _window, nullptr, &_surface)) {
            LOG_AND_THROW(std::runtime_error("create window surface"));
        }
    }

    void GraphicalEnvironment::load_shader(const std::string& file, VkShaderStageFlagBits stage) {
        _shader_modules->add_shader_module(file, stage);
    }

    void GraphicalEnvironment::init_pipeline() {
        _pipeline = std::make_unique<RayTracingPipeline>(*_device.get());
        _pipeline->init(*_shader_modules);
        std::cerr << "Pipeline initialized" << std::endl;

        frame_buffers_init();

        _command_buffer = std::make_unique<CommandBuffer>(*_device.get());
        _command_buffer->init(_surface);

        _vertex_buffer = std::make_unique<VertexBuffer>(*_device.get());
        _vertex_buffer->init(_command_buffer->command_pool());

        _uniform_buffers = std::make_unique<UniformBuffers>(*_device.get(), _settings.max_frames_in_flight);
        _uniform_buffers->init();

        synchronization_init();
    }

    void GraphicalEnvironment::frame_buffers_init() {
        auto image_views = _device->swap_chain_image_views();
        for (auto image_view : image_views) {
            _frame_buffers.emplace_back(*_device.get());
            FrameBuffer& frame_buffer = _frame_buffers.back();
            frame_buffer.init(_pipeline->render_pass(), image_view);
        }
        std::cerr << "Frame buffers initialized" << std::endl;
    }

    void GraphicalEnvironment::synchronization_init() {
        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        if (vkCreateSemaphore(_device->device(), &semaphoreInfo, nullptr, &_image_available_semaphore) != VK_SUCCESS ||
            vkCreateSemaphore(_device->device(), &semaphoreInfo, nullptr, &_render_finished_semaphore) != VK_SUCCESS ||
            vkCreateFence(_device->device(), &fenceInfo, nullptr, &_in_flight_fence) != VK_SUCCESS)
        {
            LOG_AND_THROW(std::runtime_error("failed to create synchronization objects for a frame!"));
        }
    }

    void GraphicalEnvironment::dump_device_info() const {
        VkPhysicalDeviceMemoryProperties properties;
        vkGetPhysicalDeviceMemoryProperties(_device->physical_device(), &properties);
        std::clog << "Mem types: ";
        for (int i = 0; i < properties.memoryTypeCount; ++i) {
            std::clog << std::bitset<8>(properties.memoryTypes[i].propertyFlags) << " : " << properties.memoryTypes[i].heapIndex << " ";
        }
        std::clog << std::endl;

        std::clog << "Heaps: ";
        for (int i = 0; i < properties.memoryHeapCount; ++i) {
            std::clog << std::bitset<8>(properties.memoryHeaps[i].flags) << " : " << properties.memoryHeaps[i].size << " ";
        }
        std::clog << std::endl;
    }

    void GraphicalEnvironment::start_interactive_loop() {

    }

    void GraphicalEnvironment::draw_frame() {
        vkWaitForFences(_device->device(), 1, &_in_flight_fence, VK_TRUE, UINT64_MAX);
        vkResetFences(_device->device(), 1, &_in_flight_fence);

        uint32_t imageIndex;
        vkAcquireNextImageKHR(_device->device(), _device->swap_chain(), UINT64_MAX, _image_available_semaphore, VK_NULL_HANDLE, &imageIndex);
        assert(imageIndex < _frame_buffers.size());
        auto& frame_buffer = _frame_buffers[imageIndex];

        _command_buffer->reset_record_command_buffer(_pipeline->render_pass(), frame_buffer.frame_buffer(),
                                                     _device->swap_chain_extent(), _pipeline->pipeline(),
                                                     _vertex_buffer->vertex_buffer());

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = {_image_available_semaphore};
        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;

        VkCommandBuffer command_buffers[] = { _command_buffer->command_buffer() };
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = command_buffers;

        VkSemaphore signalSemaphores[] = { _render_finished_semaphore };
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        if (vkQueueSubmit(_device->graphics_queue(), 1, &submitInfo, _in_flight_fence) != VK_SUCCESS)
        {
            LOG_AND_THROW(std::runtime_error("failed to submit draw command buffer!"));
        }

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;

        VkSwapchainKHR swapChains[] = { _device->swap_chain() };
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;

        presentInfo.pImageIndices = &imageIndex;

        vkQueuePresentKHR(_device->present_queue(), &presentInfo);
    }

} // namespace
