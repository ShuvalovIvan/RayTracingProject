#include <vulkan/vulkan_core.h>
#include <Windows.h>

#include "graphical_environment_vulkan.h"

#include <bitset>
#include <chrono>
#include <thread>

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
        _device->init(_settings, _instance, _surface, _window);

        _descriptor_set_layouts[PipelineType::Graphics] = std::make_unique<DescriptorSetLayout>(*_device.get(), PipelineType::Graphics);
        _descriptor_set_layouts[PipelineType::Graphics]->init();
        _descriptor_set_layouts[PipelineType::Compute] = std::make_unique<ComputeDescriptorSetLayout>(*_device.get(), PipelineType::Compute);
        _descriptor_set_layouts[PipelineType::Compute]->init();

        _render_pass = std::make_unique<RenderPass>(*_device);
        _render_pass->init();

        for (const auto& f : _texture_files) {
            _textures.emplace_back(std::make_unique<Texture>(*_device, f));
        }

        init_pipeline();
    }

    static void framebufferResizeCallback(GLFWwindow *window, int width, int height)
    {
        auto app = reinterpret_cast<GraphicalEnvironment *>(glfwGetWindowUserPointer(window));
        app->framebuffer_resized();
    }

    void GraphicalEnvironment::window_init() {
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

        _window = glfwCreateWindow(_settings.width, _settings.height, "Vulkan project", nullptr, nullptr);
        if (_window == nullptr)
        {
            LOG_AND_THROW(std::runtime_error("failed to create window"));
        }

        glfwSetWindowUserPointer(_window, this);
        glfwSetFramebufferSizeCallback(_window, framebufferResizeCallback);
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
        _pipelines[PipelineType::Graphics] = std::make_unique<GraphicsPipeline>(*_device.get());
        _pipelines[PipelineType::Graphics]->init(*_shader_modules, *_descriptor_set_layouts[PipelineType::Graphics], *_render_pass);
        _pipelines[PipelineType::Compute] = std::make_unique<ComputePipeline>(*_device.get());
        _pipelines[PipelineType::Compute]->init(*_shader_modules, *_descriptor_set_layouts[PipelineType::Compute], *_render_pass);
        _shader_modules.reset();
        std::clog << "Pipeline initialized" << std::endl;

        frame_buffers_init();

        _command_buffers[PipelineType::Graphics] = std::make_unique<CommandBuffers>(*_device.get(), _settings.max_frames_in_flight, PipelineType::Graphics);
        _command_buffers[PipelineType::Graphics]->init(_surface);
        _command_buffers[PipelineType::Compute] = std::make_unique<CommandBuffers>(*_device.get(), _settings.max_frames_in_flight, PipelineType::Compute);
        _command_buffers[PipelineType::Compute]->init(_surface);

        for (auto& texture : _textures) {
            texture->load(_command_buffers[PipelineType::Graphics]->command_pool());
        }

        _vertex_buffer = std::make_unique<VertexBuffer>(*_device.get());
        _vertex_buffer->init(_command_buffers[PipelineType::Graphics]->command_pool());

        _uniform_buffers = std::make_unique<UniformBuffers>(*_device.get(), _settings.max_frames_in_flight);
        _uniform_buffers->init();

        _descriptors[PipelineType::Graphics] = std::make_unique<Descriptors>(*_device.get(), _settings.max_frames_in_flight, PipelineType::Graphics);
        // Only first texture is supported for now.
        _descriptors[PipelineType::Graphics]->init(*_descriptor_set_layouts[PipelineType::Graphics], *_uniform_buffers, *_textures[0]);

        _descriptors[PipelineType::Compute] = std::make_unique<ComputeDescriptors>(*_device.get(), _settings.max_frames_in_flight, PipelineType::Compute);
        _descriptors[PipelineType::Compute]->init(*_descriptor_set_layouts[PipelineType::Compute], *_uniform_buffers, *_textures[0]);

        frames_init();
    }

    void GraphicalEnvironment::frame_buffers_init() {
        _frame_buffers.reset();
        auto image_views = _device->swap_chain_image_views();
        _frame_buffers = std::make_unique<FrameBuffers>(*_device.get(), image_views.size());
        _frame_buffers->init(*_render_pass, image_views);
        std::clog << "Frame buffers initialized" << std::endl;
    }

    void GraphicalEnvironment::frames_init() {
        for (size_t i = 0; i < _settings.max_frames_in_flight; i++)
        {
            _frames.push_back(std::make_unique<Frame>(*_device));
            _frames.back()->init();
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

    void GraphicalEnvironment::start_interactive_loop(std::chrono::milliseconds duration)
    {
        auto startTime = std::chrono::high_resolution_clock::now();

        while (!glfwWindowShouldClose(_window) &&
               (std::chrono::high_resolution_clock::now() - startTime) < duration)
        {
            glfwPollEvents();
            draw_frame();
        }

        vkDeviceWaitIdle(_device->device());
    }

    void GraphicalEnvironment::draw_frame() {
        draw_frame_computational();
        draw_frame_graphical();
    }

    void GraphicalEnvironment::draw_frame_computational() {
        PipelineType current_pipeline_type = PipelineType::Compute;
        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        assert(_current_frame < _frames.size());
        auto &current_frame = *_frames[_current_frame];
        vkWaitForFences(_device->device(), 1, &current_frame.in_flight_fence(current_pipeline_type), VK_TRUE, UINT64_MAX);

        update_uniform_buffer(_current_frame);

        vkResetFences(_device->device(), 1, &current_frame.in_flight_fence(current_pipeline_type));

        _command_buffers[current_pipeline_type]->reset_record_compute_command_buffer(
            _pipelines,
            *_descriptors[current_pipeline_type],
            _current_frame);

        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &_command_buffers[current_pipeline_type]->command_buffer(_current_frame);
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = &current_frame.render_finished_semaphore(current_pipeline_type);

        if (vkQueueSubmit(_device->compute_queue(), 1, &submitInfo, current_frame.in_flight_fence(current_pipeline_type)) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to submit compute command buffer!");
        };
    }

    void GraphicalEnvironment::draw_frame_graphical()
    {
        PipelineType current_pipeline_type = PipelineType::Graphics;
        assert(_current_frame < _frames.size());
        auto& current_frame = *_frames[_current_frame];
        vkWaitForFences(_device->device(), 1, &current_frame.in_flight_fence(current_pipeline_type), VK_TRUE, UINT64_MAX);

        uint32_t imageIndex;
        VkResult result = vkAcquireNextImageKHR(_device->device(), _device->swap_chain(), UINT64_MAX,
                                                current_frame.image_available_semaphore(), VK_NULL_HANDLE, &imageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            recreate_swap_chain();

            return;
        }
        else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        {
            LOG_AND_THROW(std::runtime_error("failed to acquire swap chain image! Err: " + std::to_string(result)));
        }

        update_backgroung_color();

        vkResetFences(_device->device(), 1, &current_frame.in_flight_fence(current_pipeline_type));

        assert(_current_frame < static_cast<size_t>(_frame_buffers->size()));
        _command_buffers[current_pipeline_type]->reset_record_graphics_command_buffer(
            _frame_buffers->frame_buffers()[imageIndex],
            _device->swap_chain_extent(),
            _pipelines,
            *_vertex_buffer,
            *_descriptors[current_pipeline_type],
            _current_frame,
            _background,
            *_render_pass);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = {current_frame.image_available_semaphore()};
        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;

        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &_command_buffers[current_pipeline_type]->command_buffer(_current_frame);

        VkSemaphore signalSemaphores[] = {current_frame.render_finished_semaphore(current_pipeline_type)};
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        if (vkQueueSubmit(_device->graphics_queue(), 1, &submitInfo, current_frame.in_flight_fence(current_pipeline_type)) != VK_SUCCESS)
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
            LOG_AND_THROW(std::runtime_error("failed to present swap chain image!"));
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
        ubo.proj = glm::perspective(
            glm::radians(45.0f),
            (float)_device->swap_chain_extent().width / (float)_device->swap_chain_extent().height, 0.1f, 10.0f);
        // The GLM shipped with Vulkan already has the compatible inversion in place, so the axis flip
        // "ubo.proj[1][1] *= -1;" mentioned in the tutorial is not necessary.

        _uniform_buffers->copy_to_uniform_buffers_for_image(currentImage, ubo);
    }

    void GraphicalEnvironment::update_backgroung_color() {
        static auto startTime = std::chrono::high_resolution_clock::now();

        auto currentTime = std::chrono::high_resolution_clock::now();
        uint64_t diff_millis = static_cast<uint64_t>(
            std::chrono::duration<float, std::chrono::milliseconds::period>(currentTime - startTime).count());

        _background.color.float32[1] = 0.0001 * (diff_millis % 65);
        _background.color.float32[2] = 0.0001 * (diff_millis % 42);
    }

    void GraphicalEnvironment::recreate_swap_chain() {
        std::clog << "Recreate swap chain" << std::endl;
        vkDeviceWaitIdle(_device->device());

        _device->cleanup_swap_chain();
        _frame_buffers.reset();

        _device->init_swap_chain(_settings, _surface, _window);
        _device->init_image_views();
        frame_buffers_init();
    }

} // namespace
