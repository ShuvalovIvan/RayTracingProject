#pragma once

#include <vulkan/vulkan.h>

#include "device.h"
#include "descriptors.h"
#include "ray_tracing_pipeline.h"
#include "vertex_buffer.h"

namespace VulkanImpl
{

    class CommandBuffers
    {
    public:
        CommandBuffers(const Device& device, int size) : _device(device), _size(size) {}

        ~CommandBuffers() {
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

            _command_buffers.resize(_size);

            VkCommandBufferAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            allocInfo.commandPool = _command_pool;
            allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            allocInfo.commandBufferCount = (uint32_t)_command_buffers.size();

            if (vkAllocateCommandBuffers(_device.device(), &allocInfo, _command_buffers.data()) != VK_SUCCESS)
            {
                LOG_AND_THROW(std::runtime_error("failed to allocate command buffers!"));
            }
        }

        VkCommandBuffer command_buffer(int index) const
        {
            return _command_buffers[index];
        }

        VkCommandBuffer& command_buffer(int index) {
            return _command_buffers[index];
        }

        VkCommandPool command_pool() const
        {
            return _command_pool;
        }

        void CommandBuffers::reset_record_command_buffer(VkFramebuffer frame_buffer,
                                                         VkExtent2D swap_chain_extent,
                                                         const RayTracingPipeline &pipeline,
                                                         const VertexBuffer &vertex_buffer,
                                                         Descriptors &descriptors,
                                                         uint32_t image_index,
                                                         uint32_t current_frame);

    private:
        const Device& _device;
        const int _size;

        VkCommandPool _command_pool;
        std::vector<VkCommandBuffer> _command_buffers;
    };

} // namespace