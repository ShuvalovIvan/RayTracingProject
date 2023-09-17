#pragma once

#include <assert.h>
#include <optional>
#include <vector>

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

#include "graphical_environment.h"

namespace VulkanImpl {

struct QueueFamilyIndices
{
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete()
    {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

class Device {
public:
    Device() {}
    ~Device() {
        for (auto image_view : _swap_chain_image_views)
        {
            vkDestroyImageView(_device, image_view, nullptr);
        }
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
        assert(_swap_chain_image_format != VK_FORMAT_UNDEFINED);
        return _swap_chain_image_format;
    }

    VkSwapchainKHR swap_chain() const {
        return _swap_chain;
    }

    VkExtent2D swap_chain_extent() const {
        return _swap_chain_extent;
    }

    std::vector<VkImageView> swap_chain_image_views() const {
        return _swap_chain_image_views;
    }

    VkQueue queue() const {
        return _queue;
    }

    VkQueue present_queue() {
        return _present_queue;
    }

    QueueFamilyIndices findQueueFamilies(VkSurfaceKHR surface) const;

private:
    void init_physical_device(VkInstance instance, VkSurfaceKHR surface);

    void init_logical_device(VkSurfaceKHR surface);

    void init_swap_chain(VkSurfaceKHR surface, GLFWwindow* window);

    void init_image_views();

	VkDevice _device = VK_NULL_HANDLE;
    VkPhysicalDevice _physical_device = VK_NULL_HANDLE;
    VkQueue _queue = VK_NULL_HANDLE;
    VkQueue _present_queue = VK_NULL_HANDLE;
    uint32_t _queue_family_index = 0;
    VkSwapchainKHR _swap_chain = VK_NULL_HANDLE;
    std::vector<VkImage> _swap_chain_images;
    VkFormat _swap_chain_image_format = VK_FORMAT_UNDEFINED;
    VkExtent2D _swap_chain_extent;
    std::vector<VkImageView> _swap_chain_image_views;
};

}  // namespace
