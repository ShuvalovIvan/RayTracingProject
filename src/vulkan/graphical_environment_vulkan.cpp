#include <vulkan/vulkan_core.h>
#include <Windows.h>

#include "graphical_environment_vulkan.h"

#include <bitset>
#include <chrono>
#include <thread>

#include "shader_loader.h"


namespace VulkanImpl
{
    using namespace RayTracingProject;

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

        _render_pass = std::make_unique<RenderPass>(*_device);
        _render_pass->init();

        for (const auto& f : _texture_files) {
            _textures.emplace_back(std::make_unique<Texture>(*_device, f, BindingKey::PrimaryTexture));
        }

        _uniform_buffers = std::make_unique<UniformBuffers>(*_device.get(), _settings.max_frames_in_flight);
        _uniform_buffers->add<UniformBufferObject>(BindingKey::CommonUBO);

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
        _command_buffers[PipelineType::Graphics] = std::make_unique<CommandBuffers>(*_device.get(), _settings.max_frames_in_flight, PipelineType::Graphics);
        _command_buffers[PipelineType::Graphics]->init(_surface);
        _command_buffers[PipelineType::Compute] = std::make_unique<CommandBuffers>(*_device.get(), _settings.max_frames_in_flight, PipelineType::Compute);
        _command_buffers[PipelineType::Compute]->init(_surface);

        for (auto &texture : _textures)
        {
            texture->load(_command_buffers[PipelineType::Graphics]->graphics_command_pool());
        }

        frame_buffers_init();

        _descriptors_manager = std::make_unique<DescriptorsManager>(*_device);
        _descriptors_manager->init(_textures, *_uniform_buffers);

        _pipelines[PipelineType::Graphics] = std::make_unique<GraphicsPipeline>(*_device.get());
        _pipelines[PipelineType::Graphics]->init(*_shader_modules, _descriptors_manager->descriptor_set_layout(), *_render_pass);
        _pipelines[PipelineType::Compute] = std::make_unique<ComputePipeline>(*_device.get());
        _pipelines[PipelineType::Compute]->init(*_shader_modules, _descriptors_manager->descriptor_set_layout(), *_render_pass);
        _shader_modules.reset();
        std::clog << "Pipeline initialized" << std::endl;

        _vertex_buffer = std::make_unique<VertexBuffer>(*_device.get());
        _vertex_buffer->init(_command_buffers[PipelineType::Graphics]->graphics_command_pool());

        _spheres_buffer = std::make_unique<DataBuffer<Sphere, 200>>(*_device);
        _spheres_buffer->init(_command_buffers[PipelineType::Compute]->compute_command_pool());

        frames_init();
    }

    void GraphicalEnvironment::frame_buffers_init() {
        _frame_buffers.reset();
        _frame_buffers = std::make_unique<FrameBuffers>(*_device.get(), _device->swap_chain_image_count());
        _frame_buffers->init(*_render_pass);
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
        ImageIndex imageIndex = draw_frame_computational();
        draw_frame_graphical(imageIndex);
    }

    ImageIndex GraphicalEnvironment::draw_frame_computational() {
        PipelineType current_pipeline_type = PipelineType::Compute;
        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        assert(_current_frame < _frames.size());
        auto &current_frame = *_frames[_current_frame];
        vkWaitForFences(_device->device(), 1, &current_frame.in_flight_fence(current_pipeline_type), VK_TRUE, UINT64_MAX);

        uint32_t imageIndex;
        VkResult result = vkAcquireNextImageKHR(_device->device(), _device->swap_chain(), UINT64_MAX,
                                                current_frame.image_available_semaphore(), VK_NULL_HANDLE, &imageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            recreate_swap_chain();

            return ImageIndex(imageIndex);
        }
        else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        {
            LOG_AND_THROW(std::runtime_error("failed to acquire swap chain image! Err: " + std::to_string(result)));
        }

        update_uniform_buffer(FrameIndex(_current_frame));

        vkResetFences(_device->device(), 1, &current_frame.in_flight_fence(current_pipeline_type));

        _command_buffers[current_pipeline_type]->reset_record_compute_command_buffer(
            _pipelines,
            *_descriptors_manager,
            FrameIndex(_current_frame),
            ImageIndex(imageIndex));

        _command_buffers[current_pipeline_type]->prepare_to_trace_barrier(FrameIndex(_current_frame),
                                                                          _device->swap_chain_image(ImageIndex(imageIndex)));
        _command_buffers[current_pipeline_type]->dispatch_raytrace(
            _pipelines, *_descriptors_manager,
            FrameIndex(_current_frame), ImageIndex(imageIndex));
        _command_buffers[current_pipeline_type]->prepare_to_present_barrier(FrameIndex(_current_frame), _device->swap_chain_image(ImageIndex(imageIndex)));
        _command_buffers[current_pipeline_type]->end_command_buffer(FrameIndex(_current_frame));

        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &_command_buffers[current_pipeline_type]->command_buffer(_current_frame);
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = &current_frame.render_finished_semaphore(current_pipeline_type);

        if (vkQueueSubmit(_device->compute_queue(), 1, &submitInfo, current_frame.in_flight_fence(current_pipeline_type)) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to submit compute command buffer!");
        };
        return ImageIndex(imageIndex);
    }

    void GraphicalEnvironment::draw_frame_graphical(ImageIndex imageIndex)
    {
        PipelineType current_pipeline_type = PipelineType::Graphics;
        assert(_current_frame < _frames.size());
        auto& current_frame = *_frames[_current_frame];
        vkWaitForFences(_device->device(), 1, &current_frame.in_flight_fence(current_pipeline_type), VK_TRUE, UINT64_MAX);

        // update_backgroung_color();

        vkResetFences(_device->device(), 1, &current_frame.in_flight_fence(current_pipeline_type));

        assert(_current_frame < static_cast<size_t>(_frame_buffers->size()));
        _command_buffers[current_pipeline_type]->reset_record_graphics_command_buffer(
            _frame_buffers->frame_buffers()[static_cast<size_t>(imageIndex)],
            _device->swap_chain_extent(),
            _pipelines,
            *_vertex_buffer,
            *_descriptors_manager,
            FrameIndex(_current_frame),
            ImageIndex(imageIndex),
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

        uint32_t intImageIndex = static_cast<uint32_t>(imageIndex);
        presentInfo.pImageIndices = &intImageIndex;

        VkResult result = vkQueuePresentKHR(_device->present_queue(), &presentInfo);

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

    void GraphicalEnvironment::update_uniform_buffer(FrameIndex current_frame)
    {
        static auto startTime = std::chrono::high_resolution_clock::now();

        auto currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

        UniformBufferObject ubo{};
        ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, -2.0f) /*eye*/, glm::vec3(0.0f, 0.0f, 0.0f) /*center*/,
            glm::vec3(0.0f, 1.0f, 0.0f) /*up*/);
        ubo.proj = glm::perspective(
            glm::radians(45.0f),
            (float)_device->swap_chain_extent().width / (float)_device->swap_chain_extent().height, 0.1f, 10.0f);
        ubo.proj[1][1] *= -1;

        _uniform_buffers->copy_to_uniform_buffers_for_frame(current_frame, BindingKey::CommonUBO, ubo);
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

    void GraphicalEnvironment::add_spheres(const std::vector<RayTracingProject::Sphere> &spheres)
    {
        for (const auto& s : spheres) {
            _spheres_buffer->append(s);
        }
    }

} // namespace
