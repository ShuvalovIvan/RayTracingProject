
#pragma once

#include <vulkan/vulkan.h>

#include <iostream>
#include <vector>

namespace VulkanImpl {

class Validation {
public:
    Validation() {}

    ~Validation();

    void init(VkInstance instance, bool validation_required = true);

    void append_extensions(std::vector<const char*> *extensions);

    std::vector<const char*> supported_layers();

private:
    bool check_validation_layer_support();

    const std::vector<const char *> _validation_layers = {
        "VK_LAYER_KHRONOS_validation",
        "VK_LAYER_LUNARG_monitor",
        "VK_LAYER_LUNARG_screenshot",
        "VK_LAYER_KHRONOS_shader_object"};

    bool _enabled = false;
    VkInstance _instance;
    VkDebugUtilsMessengerEXT _debug_messenger = VK_NULL_HANDLE;
};

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
    void *pUserData)
{
    std::clog << "validation layer: " << pCallbackData->pMessage << std::endl;

    return VK_FALSE;
}

}  // namespace
