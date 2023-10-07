#include <vulkan/vulkan_core.h>
#include <Windows.h>

#include "graphical_environment_vulkan.h"

#include <bitset>
#include <chrono>

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

        window_init();
        std::clog << "Window initialized" << std::endl;

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

        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
        debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        debugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        debugCreateInfo.pfnUserCallback = debugCallback;

        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT *)&debugCreateInfo;

        if (VK_SUCCESS != vkCreateInstance(&createInfo, nullptr, &_instance)) {
            LOG_AND_THROW(std::runtime_error("create instance"));
        }
        std::clog << "Instance created" << std::endl;
        if (_validation != nullptr) {
            _validation->init(_instance);
        }

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

        _window = glfwCreateWindow(1024, 768, "Vulkan project", nullptr, nullptr);
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
        _pipeline->init(*_shader_modules, *_descriptor_set_layout);
        _shader_modules.reset();
        std::clog << "Pipeline initialized" << std::endl;

        frame_buffers_init();

        _command_buffers = std::make_unique<CommandBuffers>(*_device.get(), _settings.max_frames_in_flight);
        _command_buffers->init(_surface);

        for (auto& texture : _textures) {
            texture.load(_command_buffers->command_pool());
        }

        _vertex_buffer = std::make_unique<VertexBuffer>(*_device.get());
        _vertex_buffer->init(_command_buffers->command_pool());

        _uniform_buffers = std::make_unique<UniformBuffers>(*_device.get(), _settings.max_frames_in_flight);
        _uniform_buffers->init();

        _descriptors = std::make_unique<Descriptors>(*_device.get(), _settings.max_frames_in_flight);
        // Only first texture is supported for now.
        _descriptors->init(*_descriptor_set_layout, *_uniform_buffers, _textures[0]);

        synchronization_init();
    }

    void GraphicalEnvironment::frame_buffers_init() {
        _frame_buffers.reset();
        auto image_views = _device->swap_chain_image_views();
        _frame_buffers = std::make_unique<FrameBuffers>(*_device.get(), image_views.size());
        _frame_buffers->init(_pipeline->render_pass(), image_views);
        std::clog << "Frame buffers initialized" << std::endl;
    }

    void GraphicalEnvironment::synchronization_init() {
        _image_available_semaphores.resize(_settings.max_frames_in_flight);
        _render_finished_semaphores.resize(_settings.max_frames_in_flight);
        _in_flight_fences.resize(_settings.max_frames_in_flight);

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (size_t i = 0; i < _settings.max_frames_in_flight; i++)
        {
            if (vkCreateSemaphore(_device->device(), &semaphoreInfo, nullptr, &_image_available_semaphores[i]) != VK_SUCCESS ||
                vkCreateSemaphore(_device->device(), &semaphoreInfo, nullptr, &_render_finished_semaphores[i]) != VK_SUCCESS ||
                vkCreateFence(_device->device(), &fenceInfo, nullptr, &_in_flight_fences[i]) != VK_SUCCESS)
            {
                LOG_AND_THROW(std::runtime_error("failed to create synchronization objects for a frame!"));
            }
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
        assert(_current_frame < _in_flight_fences.size());
        vkWaitForFences(_device->device(), 1, &_in_flight_fences[_current_frame], VK_TRUE, UINT64_MAX);

        uint32_t imageIndex;
        assert(_current_frame < _image_available_semaphores.size());
        VkResult result = vkAcquireNextImageKHR(_device->device(), _device->swap_chain(), UINT64_MAX,
                                                _image_available_semaphores[_current_frame], VK_NULL_HANDLE, &imageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            recreate_swap_chain();
            return;
        }
        else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        {
            LOG_AND_THROW(std::runtime_error("failed to acquire swap chain image!"));
        }

        update_uniform_buffer(_current_frame);

        vkResetFences(_device->device(), 1, &_in_flight_fences[_current_frame]);

        assert(_current_frame < static_cast<size_t>(_frame_buffers->size()));
        _command_buffers->reset_record_command_buffer(_frame_buffers->frame_buffers()[imageIndex],
                                                      _device->swap_chain_extent(),
                                                      *_pipeline,
                                                      *_vertex_buffer,
                                                      *_descriptors,
                                                      _current_frame);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = {_image_available_semaphores[_current_frame]};
        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;

        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &_command_buffers->command_buffer(_current_frame);

        assert(_current_frame < _render_finished_semaphores.size());
        VkSemaphore signalSemaphores[] = {_render_finished_semaphores[_current_frame]};
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        if (vkQueueSubmit(_device->graphics_queue(), 1, &submitInfo, _in_flight_fences[_current_frame]) != VK_SUCCESS)
        {
            LOG_AND_THROW(std::runtime_error("failed to submit draw command buffer!"));
        }

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;

        VkSwapchainKHR swapChains[] = {_device->swap_chain()};
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;

        presentInfo.pImageIndices = &imageIndex;

        result = vkQueuePresentKHR(_device->present_queue(), &presentInfo);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || _framebuffer_resized)
        {
            _framebuffer_resized = false;
            recreate_swap_chain();
        }
        else if (result != VK_SUCCESS)
        {
            throw std::runtime_error("failed to present swap chain image!");
        }

        _current_frame = (_current_frame + 1) % _settings.max_frames_in_flight;
    }

    void GraphicalEnvironment::update_uniform_buffer(uint32_t currentImage)
    {
        static auto startTime = std::chrono::high_resolution_clock::now();

        auto currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

        UniformBufferObject ubo{};
        ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        ubo.proj = glm::perspective(glm::radians(45.0f), _device->swap_chain_extent().width / (float)_device->swap_chain_extent().height, 0.1f, 10.0f);
        ubo.proj[1][1] *= -1;

        _uniform_buffers->copy_to_uniform_buffers_for_image(currentImage, ubo);
    }

    void GraphicalEnvironment::recreate_swap_chain() {
        std::clog << "Recreate swap chain" << std::endl;
        vkDeviceWaitIdle(_device->device());

        _device->cleanup_swap_chain();
        _frame_buffers.reset();

        _device->init_swap_chain(_surface, _window);
        _device->init_image_views();
        frame_buffers_init();
    }

} // namespace
