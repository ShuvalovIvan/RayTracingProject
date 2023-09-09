#pragma once

#include <vector>

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

#include "graphical_environment.h"


namespace VulkanImpl {

class Device {
public:
    Device() {}
    ~Device() {
        vkDestroySwapchainKHR(_device, _swap_chain, nullptr);
        vkDestroyDevice(_device, 0);
        std::clog << "Device destroyed" << std::endl;
    }

    void init(VkInstance instance, VkSurfaceKHR surface, GLFWwindow* window);

    VkDevice device() const {
        return _device;
    }

    VkPhysicalDevice physical_device() const {
        return _physical_device;
    }

    VkFormat format() const {
        return _swap_chain_image_format;
    }

    VkExtent2D swap_chain_extent() const {
        return _swap_chain_extent;
    }

private:
    void init_physical_device(VkInstance instance, VkSurfaceKHR surface);

    void init_logical_device();

    void init_swap_chain(VkSurfaceKHR surface, GLFWwindow* window);

	VkDevice _device = VK_NULL_HANDLE;
    VkPhysicalDevice _physical_device = VK_NULL_HANDLE;
    VkQueue _queue = VK_NULL_HANDLE;
    uint32_t _queue_family_index = 0;
    VkSwapchainKHR _swap_chain = VK_NULL_HANDLE;
    std::vector<VkImage> _swap_chain_images;
    VkFormat _swap_chain_image_format = VK_FORMAT_UNDEFINED;
    VkExtent2D _swap_chain_extent;
};

}  // namespace
