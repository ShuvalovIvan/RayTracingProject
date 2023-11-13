#pragma once

#include <assert.h>
#include <optional>
#include <set>
#include <vector>

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

#include "graphical_environment.h"
#include "vulkan_common_objects.h"

namespace VulkanImpl {

struct QueueFamilyIndices
{
    std::optional<uint32_t> computeFamily;
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool separate_compute_family() const {
        return computeFamily != graphicsFamily;
    }

    std::vector<uint32_t> unique_indices() const {
        std::set<uint32_t> unique{ computeFamily.value(), graphicsFamily.value(), presentFamily.value() };
        std::vector<uint32_t> result;
        std::copy(unique.begin(), unique.end(), std::back_inserter(result));
        return result;
    }

    bool isComplete() const
    {
        return graphicsFamily.has_value() && computeFamily.has_value() && presentFamily.has_value();
    }

    void describe() const {
        std::clog << "Compute: " << *computeFamily << ", graphics: " << *graphicsFamily << std::endl;
    }
};

struct SwapChainSupportDetails
{
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

enum class PipelineType {
    Graphics,
    Compute
};

enum class ImageIndex : uint32_t;
enum class ImagesCount : uint32_t;

class Device {
public:
    Device() {}
    ~Device() {
        vkDeviceWaitIdle(_device);
        cleanup_swap_chain();
        vkDestroyDevice(_device, 0);
        std::clog << "Device destroyed" << std::endl;
    }

    void init(const RayTracingProject::GraphicalEnvironmentSettings &settings, VkInstance instance,
              VkSurfaceKHR surface, GLFWwindow *const window);

    void init_swap_chain(const RayTracingProject::GraphicalEnvironmentSettings &settings,
                         VkSurfaceKHR surface, GLFWwindow *window);

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

    ImagesCount swap_chain_image_count() const {
        assert(!_swap_chain_images.empty());
        return ImagesCount(_swap_chain_images.size());
    }

    VkImageView swap_chain_image_view(ImageIndex image_index) const {
        uint32_t i = static_cast<uint32_t>(image_index);
        assert(i < _swap_chain_image_views.size());
        return _swap_chain_image_views[i];
    }

    VkImage swap_chain_image(ImageIndex image_index) const
    {
        uint32_t i = static_cast<uint32_t>(image_index);
        assert(i < _swap_chain_images.size());
        return _swap_chain_images[i];
    }

    VkQueue graphics_queue() const {
        return _graphics_queue;
    }

    VkQueue compute_queue() const {
        return _compute_queue;
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
    VkQueue _compute_queue = VK_NULL_HANDLE;
    VkQueue _present_queue = VK_NULL_HANDLE;
    VkSwapchainKHR _swap_chain = VK_NULL_HANDLE;
    std::vector<VkImage> _swap_chain_images;
    VkFormat _swap_chain_image_format = VK_FORMAT_UNDEFINED;
    VkExtent2D _swap_chain_extent;
    std::vector<VkImageView> _swap_chain_image_views;
};

}  // namespace
