#pragma once

#include <vulkan/vulkan.h>
#include <map>

#include "device.h"
#include "descriptors_manager.h"
#include "frame_buffers.h"
#include "ray_tracing_pipeline.h"
#include "vertex_buffer.h"

namespace VulkanImpl
{

    class CommandBuffers
    {
    public:
        CommandBuffers(const Device& device, int size, PipelineType type) : _device(device), _size(size), _type(type) {}

        ~CommandBuffers() {
            for (auto cb : _command_buffers)
            {
                auto result = vkResetCommandBuffer(cb, /*VkCommandBufferResetFlagBits*/ 0);
                if (result != VK_SUCCESS)
                {
                    std::cerr << "Command buffer reset error " << result << std::endl;
                }
            }
            for (auto cb : _compute_command_buffers)
            {
                auto result = vkResetCommandBuffer(cb, /*VkCommandBufferResetFlagBits*/ 0);
                if (result != VK_SUCCESS)
                {
                    std::cerr << "Command buffer reset error " << result << std::endl;
                }
            }
            _command_buffers.clear();
            _compute_command_buffers.clear();
            vkDestroyCommandPool(_device.device(), _graphics_command_pool, nullptr);
            vkDestroyCommandPool(_device.device(), _compute_command_pool, nullptr);
        }

        void init(VkSurfaceKHR surface) {
            QueueFamilyIndices queueFamilyIndices = _device.findQueueFamilies(surface);

            VkCommandPoolCreateInfo poolInfo{};
            poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
            poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
            poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

            if (vkCreateCommandPool(_device.device(), &poolInfo, nullptr, &_graphics_command_pool) != VK_SUCCESS)
            {
                LOG_AND_THROW(std::runtime_error("failed to create command pool!"));
            }

            _command_buffers.resize(_size);

            VkCommandBufferAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            allocInfo.commandPool = _graphics_command_pool;
            allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            allocInfo.commandBufferCount = (uint32_t)_command_buffers.size();

            if (vkAllocateCommandBuffers(_device.device(), &allocInfo, _command_buffers.data()) != VK_SUCCESS)
            {
                LOG_AND_THROW(std::runtime_error("failed to allocate command buffers!"));
            }

            // Separate compute command pool.
            VkCommandPoolCreateInfo cmdPoolInfo = {};
            cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
            cmdPoolInfo.queueFamilyIndex = queueFamilyIndices.computeFamily.value();
            cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
            if (vkCreateCommandPool(_device.device(), &cmdPoolInfo, nullptr, &_compute_command_pool) != VK_SUCCESS)
            {
                LOG_AND_THROW(std::runtime_error("failed to create command pool!"));
            }
        }

        PipelineType type() const {
            return _type;
        }

        VkCommandBuffer command_buffer(int index) const
        {
            assert(index < _command_buffers.size());
            return _command_buffers[index];
        }

        VkCommandBuffer& command_buffer(int index) {
            assert(index < _command_buffers.size());
            return _command_buffers[index];
        }

        VkCommandPool graphics_command_pool() const
        {
            return _graphics_command_pool;
        }

        VkCommandPool compute_command_pool() const {
            return _compute_command_pool;
        }

        void reset_record_graphics_command_buffer(VkFramebuffer frame_buffer,
                                                  VkExtent2D swap_chain_extent,
                                                  std::map<PipelineType, std::unique_ptr<Pipeline>> &pipelines,
                                                  const VertexBuffer &vertex_buffer,
                                                  DescriptorsManager &descriptors,
                                                  FrameIndex current_frame,
                                                  ImageIndex image_index,
                                                  VkClearValue background,
                                                  const RenderPass &render_pass);

        void reset_record_compute_command_buffer(std::map<PipelineType, std::unique_ptr<Pipeline>> &pipelines,
                                                 DescriptorsManager &descriptors,
                                                 FrameIndex current_frame,
                                                 ImageIndex image_index);

        void prepare_to_trace_barrier(FrameIndex current_frame, VkImage image);
        void dispatch_raytrace(std::map<PipelineType, std::unique_ptr<Pipeline>> &pipelines,
                               DescriptorsManager &descriptors,
                               FrameIndex current_frame, ImageIndex image_index);
        void prepare_to_present_barrier(FrameIndex current_frame, VkImage image);
        void end_command_buffer(FrameIndex current_frame);

    private:
        CommandBuffers(const CommandBuffers &) = delete;
        CommandBuffers &operator=(const CommandBuffers &) = delete;

        const Device &_device;
        const int _size;
        const PipelineType _type;

        VkCommandPool _graphics_command_pool;
        VkCommandPool _compute_command_pool;
        std::vector<VkCommandBuffer> _command_buffers;
        std::vector<VkCommandBuffer> _compute_command_buffers;
    };

} // namespace
