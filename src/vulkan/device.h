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

struct SwapChainSupportDetails
{
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

class Device {
public:
    Device() {}
    ~Device() {
        vkDeviceWaitIdle(_device);
        cleanup_swap_chain();
        vkDestroyDevice(_device, 0);
        std::clog << "Device destroyed" << std::endl;
    }

    void init(VkInstance instance, VkSurfaceKHR surface, GLFWwindow* window);

    void init_swap_chain(VkSurfaceKHR surface, GLFWwindow *window);

    void init_image_views();

    void cleanup_swap_chain() {
        for (auto image_view : _swap_chain_image_views)
        {
            vkDestroyImageView(_device, image_view, nullptr);
        }
        _swap_chain_image_views.clear();

        if (_swap_chain != VK_NULL_HANDLE) {
            vkDestroySwapchainKHR(_device, _swap_chain, nullptr);
            _swap_chain = VK_NULL_HANDLE;
        }
    }

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

    VkQueue graphics_queue() const {
        return _graphics_queue;
    }

    VkQueue present_queue() {
        return _present_queue;
    }

    static SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);

    QueueFamilyIndices findQueueFamilies(VkSurfaceKHR surface) const {
        return Device::findQueueFamilies(_physical_device, surface);
    }

    static QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);

    VkImageView createImageView(VkImage image, VkFormat format);

private:
    Device(const Device &) = delete;
    Device &operator=(const Device &) = delete;

    void init_physical_device(VkInstance instance, VkSurfaceKHR surface);

    void init_logical_device(VkSurfaceKHR surface);

    VkDevice _device = VK_NULL_HANDLE;
    VkPhysicalDevice _physical_device = VK_NULL_HANDLE;
    VkQueue _graphics_queue = VK_NULL_HANDLE;
    VkQueue _present_queue = VK_NULL_HANDLE;
    VkSwapchainKHR _swap_chain = VK_NULL_HANDLE;
    std::vector<VkImage> _swap_chain_images;
    VkFormat _swap_chain_image_format = VK_FORMAT_UNDEFINED;
    VkExtent2D _swap_chain_extent;
    std::vector<VkImageView> _swap_chain_image_views;
};

}  // namespace
