#include <stdexcept>

#include "vulkan/device.h"

namespace VulkanImpl {

static constexpr int MAX_DEVICE_COUNT = 10;
static constexpr int MAX_QUEUE_COUNT = 4;


void Device::init(VkInstance instance, VkSurfaceKHR surface) {
    init_physical_device(instance, surface);
}

void Device::init_physical_device(VkInstance instance, VkSurfaceKHR surface) {
    uint32_t physicalDeviceCount;
    VkPhysicalDevice deviceHandles[MAX_DEVICE_COUNT];
    VkQueueFamilyProperties queueFamilyProperties[MAX_QUEUE_COUNT];
    uint32_t queueFamilyIndex;
    VkQueue queue;

    vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, 0);
    physicalDeviceCount = physicalDeviceCount > MAX_DEVICE_COUNT ? MAX_DEVICE_COUNT : physicalDeviceCount;
    vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, deviceHandles);

    for (uint32_t i = 0; i < physicalDeviceCount; ++i)
    {
        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(deviceHandles[i], &queueFamilyCount, NULL);
        queueFamilyCount = queueFamilyCount > MAX_QUEUE_COUNT ? MAX_QUEUE_COUNT : queueFamilyCount;
        vkGetPhysicalDeviceQueueFamilyProperties(deviceHandles[i], &queueFamilyCount, queueFamilyProperties);

        for (uint32_t j = 0; j < queueFamilyCount; ++j) {

            VkBool32 supportsPresent = VK_FALSE;
            vkGetPhysicalDeviceSurfaceSupportKHR(deviceHandles[i], j, surface, &supportsPresent);

            if (supportsPresent && (queueFamilyProperties[j].queueFlags & VK_QUEUE_GRAPHICS_BIT))
            {
                queueFamilyIndex = j;
                _physicalDevice = deviceHandles[i];
                break;
            }
        }

        if (_physicalDevice)
        {
            break;
        }
    }

    if (_physicalDevice == nullptr) {
        throw std::runtime_error("Physical device not found");
    }
}

}  // namespace
