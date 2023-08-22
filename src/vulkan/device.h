#pragma once

#include <vulkan/vulkan.h>

namespace VulkanImpl {

class Device {
public:
    Device() {}

    void init(VkInstance instance, VkSurfaceKHR surface);

    VkDevice device() const {
        return _device;
    }

private:
    void init_physical_device(VkInstance instance, VkSurfaceKHR surface);

	VkDevice _device = VK_NULL_HANDLE;
    VkPhysicalDevice _physicalDevice = VK_NULL_HANDLE;
};

}  // namespace
