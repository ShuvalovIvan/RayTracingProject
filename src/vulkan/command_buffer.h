#pragma once

#include <vulkan/vulkan.h>

#include "device.h"

namespace VulkanImpl
{

    class CommandBuffer
    {
    public:
        CommandBuffer(const Device& device) : _device(device) {}

        ~CommandBuffer() {
            vkDestroyCommandPool(_device.device(), _command_pool, nullptr);
        }

        void init(VkSurfaceKHR surface) {
            QueueFamilyIndices queueFamilyIndices = _device.findQueueFamilies(surface);

            VkCommandPoolCreateInfo poolInfo{};
            poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
            poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
            poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

            if (vkCreateCommandPool(_device.device(), &poolInfo, nullptr, &_command_pool) != VK_SUCCESS)
            {
                LOG_AND_THROW(std::runtime_error("failed to create command pool!"));
            }

            VkCommandBufferAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            allocInfo.commandPool = _command_pool;
            allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            allocInfo.commandBufferCount = 1;

            if (vkAllocateCommandBuffers(_device.device(), &allocInfo, &_command_buffer) != VK_SUCCESS)
            {
                LOG_AND_THROW(std::runtime_error("failed to allocate command buffers!"));
            }
        }

        VkCommandBuffer command_buffer() const {
            return _command_buffer;
        }

        void CommandBuffer::reset_record_command_buffer(VkRenderPass render_pass,
                                                        VkFramebuffer frame_buffer,
                                                        VkExtent2D swap_chain_extent,
                                                        VkPipeline pipeline);

    private:
        const Device& _device;
        VkCommandPool _command_pool;
        VkCommandBuffer _command_buffer = VK_NULL_HANDLE;
    };

} // namespace
