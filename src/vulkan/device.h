#pragma once

#include <vulkan/vulkan.h>

namespace VulkanImpl {

class Device {
public:
    Device() {}
    ~Device() {
        vkDestroyDevice(_device, 0);
    }

    void init(VkInstance instance, VkSurfaceKHR surface);

    VkDevice device() const {
        return _device;
    }

private:
    void init_physical_device(VkInstance instance, VkSurfaceKHR surface);

    void init_logical_device();

	VkDevice _device = VK_NULL_HANDLE;
    VkPhysicalDevice _physical_device = VK_NULL_HANDLE;
    VkQueue _queue = VK_NULL_HANDLE;
    uint32_t _queue_family_index = 0;
};

}  // namespace
