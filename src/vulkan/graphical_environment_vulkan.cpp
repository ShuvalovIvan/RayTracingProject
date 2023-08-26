#include <vulkan/vulkan_core.h>
#include <Windows.h>

#include "graphical_environment_vulkan.h"

#include "shader_loader.h"


namespace VulkanImpl
{

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
        std::clog << "Extensions found with GLFW: " << glfwExtensionCount << " ";
        std::copy(&glfwExtensions[0], &glfwExtensions[glfwExtensionCount], std::ostream_iterator<const char *>(std::clog, "\n"));

        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;
        createInfo.enabledExtensionCount = glfwExtensionCount;
        createInfo.ppEnabledExtensionNames = glfwExtensions;

        if (VK_SUCCESS != vkCreateInstance(&createInfo, nullptr, &_instance)) {
            LOG_AND_THROW(std::runtime_error("create instance"));
        }
        std::clog << "Instance created" << std::endl;

        window_init();
        std::clog << "Window initialized" << std::endl;
        surface_init();
        std::clog << "Surface initialized" << std::endl;
        _device.init(_instance, _surface, _window);
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
        ShaderLoader loader = {file, _device};
        auto shader = loader.load_shader_module(stage);
        std::clog << "shader " << file << " loaded" << std::endl;
    }

} // namespace
