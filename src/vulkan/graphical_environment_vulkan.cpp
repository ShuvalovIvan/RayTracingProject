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

        VkApplicationInfo appInfo;
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
        _shader_modules = std::make_unique<ShaderModules>(*_device);
        std::cerr << "Vulkan initialized" << std::endl;
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
    }

    void GraphicalEnvironment::frame_buffers_init() {
        auto image_views = _device->swap_chain_image_views();
        for (auto image_view : image_views) {
            FrameBuffer frame_buffer(*_device);
            frame_buffer.init(_pipeline->render_pass(), image_view);
            _frame_buffers.push_back(std::move(frame_buffer));
        }
        std::cerr << "Frame buffers initialized" << std::endl;
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

    void GraphicalEnvironment::start_loop() {

    }


} // namespace
