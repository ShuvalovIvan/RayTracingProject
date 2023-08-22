#include "graphical_environment_vulkan.h"

#include <vulkan/vulkan_core.h>

namespace VulkanImpl
{

    void GraphicalEnvironment::init()
    {
        if (!glfwInit())
        {
            throw std::runtime_error("glfwInit() failed");
        }

        if (!glfwVulkanSupported())
        {
            throw std::runtime_error("glfwVulkanSupported() failed");
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
            throw std::runtime_error("No instance extensions found");
        }

        VkInstanceCreateInfo createInfo;
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;
        createInfo.enabledExtensionCount = glfwExtensionCount;
        createInfo.ppEnabledExtensionNames = glfwExtensions;

        if (VK_SUCCESS != vkCreateInstance(&createInfo, nullptr, &_instance)) {
            throw std::runtime_error("create instance");
        }
    
        window_init();
        surface_init();
        _device.init(_instance, _surface);
    }

    void GraphicalEnvironment::window_init() {
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

        _window = glfwCreateWindow(800, 600, "Vulkan project", nullptr, nullptr);
        if (_window == nullptr)
        {
            throw std::runtime_error("failed to create window");
        }

        glfwSetWindowUserPointer(_window, this);
        // glfwSetKeyCallback(_window, GlfwKeyCallback);
        // glfwSetCursorPosCallback(window_, GlfwCursorPositionCallback);
        // glfwSetMouseButtonCallback(window_, GlfwMouseButtonCallback);
        // glfwSetScrollCallback(window_, GlfwScrollCallback);
    }

    void GraphicalEnvironment::surface_init() {
        if (VK_SUCCESS != glfwCreateWindowSurface(_instance, _window, nullptr, &_surface)) {
            throw std::runtime_error("create window surface");
        }
    }

} // namespace
