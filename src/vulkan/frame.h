#pragma once

#include <vulkan/vulkan.h>

#include <map>

#include "device.h"

namespace VulkanImpl
{

class Frame {
public:
    Frame(const Device& device) : _device(device) {}

    ~Frame() {
        vkWaitForFences(_device.device(), 1, &_in_flight_fences[PipelineType::Compute], VK_TRUE, UINT64_MAX);
        vkWaitForFences(_device.device(), 1, &_in_flight_fences[PipelineType::Graphics], VK_TRUE, UINT64_MAX);

        vkDestroySemaphore(_device.device(), _image_available_semaphore, nullptr);
        vkDestroySemaphore(_device.device(), _render_finished_semaphores[PipelineType::Compute], nullptr);
        vkDestroySemaphore(_device.device(), _render_finished_semaphores[PipelineType::Graphics], nullptr);
        vkDestroyFence(_device.device(), _in_flight_fences[PipelineType::Compute], nullptr);
        vkDestroyFence(_device.device(), _in_flight_fences[PipelineType::Graphics], nullptr);
    }

    void init() {
        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        if (vkCreateSemaphore(_device.device(), &semaphoreInfo, nullptr, &_image_available_semaphore) != VK_SUCCESS ||
            vkCreateSemaphore(_device.device(), &semaphoreInfo, nullptr, &_render_finished_semaphores[PipelineType::Compute]) != VK_SUCCESS ||
            vkCreateSemaphore(_device.device(), &semaphoreInfo, nullptr, &_render_finished_semaphores[PipelineType::Graphics]) != VK_SUCCESS)
        {
            LOG_AND_THROW(std::runtime_error("failed to create synchronization objects for a frame!"));
        }

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        if (vkCreateFence(_device.device(), &fenceInfo, nullptr, &_in_flight_fences[PipelineType::Compute]) != VK_SUCCESS ||
            vkCreateFence(_device.device(), &fenceInfo, nullptr, &_in_flight_fences[PipelineType::Graphics]) != VK_SUCCESS)
        {
            LOG_AND_THROW(std::runtime_error("failed to create synchronization objects for a frame!"));
        }
    }

    VkSemaphore& image_available_semaphore() {
        return _image_available_semaphore;
    }

    VkSemaphore& render_finished_semaphore(PipelineType type) {
        return (*_render_finished_semaphores.find(type)).second;
    }

    VkFence& in_flight_fence(PipelineType type) {
        return (*_in_flight_fences.find(type)).second;
    }

private:
    Frame(const Frame&) = delete;
    Frame &operator=(const Frame&) = delete;

    const Device& _device;

    VkSemaphore _image_available_semaphore = VK_NULL_HANDLE;
    std::map<PipelineType, VkFence> _in_flight_fences;
    std::map<PipelineType, VkSemaphore> _render_finished_semaphores;
};

} // namespace

