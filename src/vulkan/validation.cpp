#include "vulkan/validation.h"

#include <assert.h>

#include "vulkan/graphical_environment_vulkan.h"

namespace VulkanImpl {

Validation::~Validation() {
    if (_debug_messenger) {
        assert(_instance);
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(_instance, "vkDestroyDebugUtilsMessengerEXT");
        if (func != nullptr) {
            func(_instance, _debug_messenger, nullptr);
        }
        std::clog << "Validation deleted" << std::endl;
    }
}

void Validation::init(VkInstance instance, bool validation_required) {
    _instance = instance;
    if (!check_validation_layer_support()) {
        const static auto msg = "Validation is not supported";
        std::clog << msg << std::endl;
        if (validation_required) {
            LOG_AND_THROW(std::runtime_error(msg));
        }
    }
    assert(_enabled || !validation_required);

    VkDebugUtilsMessengerCreateInfoEXT createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
    createInfo.pUserData = nullptr; // Optional

    auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    VkResult init_result = VK_ERROR_EXTENSION_NOT_PRESENT;
    if (func != nullptr) {
        init_result = func(_instance, &createInfo, nullptr, &_debug_messenger);
    }

    if (init_result != VK_SUCCESS) {
        LOG_AND_THROW(std::runtime_error("Debug extension not present"));
    }
    std::clog << "Validation initialized" << std::endl;
}

void Validation::append_extensions(std::vector<const char*> *extensions) {
    extensions->push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
}

bool Validation::check_validation_layer_support() {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const auto& layer : availableLayers) {
        std::clog << layer.layerName << ": " << layer.description << std::endl;
    }

    for (const char* layerName : _validation_layers) {
        bool layerFound = false;

        for (const auto& layerProperties : availableLayers) {
            if (strcmp(layerName, layerProperties.layerName) == 0) {
                layerFound = true;
                _enabled = true;
                break;
            }
        }

        if (!layerFound) {
            return false;
        }
    }

    _enabled = true;
    return true;
}

std::vector<const char*> Validation::supported_layers() {
    if (check_validation_layer_support()) {
        return _validation_layers;
    }
    return std::vector<const char*>();
}

}  // namespace
