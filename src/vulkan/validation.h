
#pragma once

#include <vulkan/vulkan.h>

#include <vector>

namespace VulkanImpl {

class Validation {
public:
    Validation() {}

    ~Validation();

    void init(VkInstance instance, bool validation_required = true);

    void append_extensions(std::vector<const char*> *extensions);

private:
    bool check_validation_layer_support();

    const std::vector<const char*> _validation_layers = {
        "VK_LAYER_KHRONOS_validation"
    };

    bool _enabled = false;
    VkInstance _instance;
    VkDebugUtilsMessengerEXT _debug_messenger = VK_NULL_HANDLE;
};


}  // namespace
