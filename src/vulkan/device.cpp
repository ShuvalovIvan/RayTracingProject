#include <stdexcept>

#include "vulkan/device.h"

#include "graphical_environment.h"

namespace VulkanImpl {

static constexpr int MAX_DEVICE_COUNT = 10;
static constexpr int MAX_QUEUE_COUNT = 4;


void Device::init(VkInstance instance, VkSurfaceKHR surface) {
    init_physical_device(instance, surface);
    init_logical_device();
}

void Device::init_physical_device(VkInstance instance, VkSurfaceKHR surface) {
    uint32_t physicalDeviceCount;
    VkPhysicalDevice deviceHandles[MAX_DEVICE_COUNT];
    VkQueueFamilyProperties queueFamilyProperties[MAX_QUEUE_COUNT];

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
                _queue_family_index = j;
                _physical_device = deviceHandles[i];
                break;
            }
        }

        if (_physical_device)
        {
            break;
        }
    }

    if (_physical_device == nullptr) {
        LOG_AND_THROW(std::runtime_error("Physical device not found"));
    }
}

void Device::init_logical_device() {
    const char* const extension_names[] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
    VkDeviceCreateInfo deviceCreateInfo{};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.queueCreateInfoCount = 1;
    deviceCreateInfo.pQueueCreateInfos = 0;
    deviceCreateInfo.enabledLayerCount = 0;
    deviceCreateInfo.ppEnabledLayerNames = 0;
    deviceCreateInfo.enabledExtensionCount = 1;
    deviceCreateInfo.ppEnabledExtensionNames = extension_names;
    deviceCreateInfo.pEnabledFeatures = 0;

    const float priorities[] = { 1.0f };
    VkDeviceQueueCreateInfo queue_create_info[] = {
        VkDeviceQueueCreateInfo{}
    };
    queue_create_info[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_create_info[0].queueFamilyIndex = _queue_family_index;
    queue_create_info[0].queueCount = 1;
    queue_create_info[0].pQueuePriorities = priorities;

    deviceCreateInfo.pQueueCreateInfos = queue_create_info;
    VkResult result = vkCreateDevice(_physical_device, &deviceCreateInfo, nullptr, &_device);
    if (result != VK_SUCCESS) {
        LOG_AND_THROW(std::runtime_error("Logical device not created"));
    }

    vkGetDeviceQueue(_device, _queue_family_index, 0, &_queue);
}

}  // namespace
