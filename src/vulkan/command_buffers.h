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
        CommandBuffers(const Device &device, PipelineType type)
            : _device(device), _images_count(ImagesCount(device.swap_chain_image_count())), _type(type) {}

        ~CommandBuffers() {
            for (auto cb : _command_buffers)
            {
                auto result = vkResetCommandBuffer(cb, /*VkCommandBufferResetFlagBits*/ 0);
                if (result != VK_SUCCESS)
                {
                    std::cerr << "Command buffer reset error " << result << std::endl;
                }
            }
            _command_buffers.clear();
            vkDestroyCommandPool(_device.device(), _command_pool, nullptr);
        }

        void init(VkSurfaceKHR surface) {
            QueueFamilyIndices queueFamilyIndices = _device.findQueueFamilies(surface);

            if (_type == PipelineType::Graphics) {
                VkCommandPoolCreateInfo poolInfo{};
                poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
                poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
                poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

                if (vkCreateCommandPool(_device.device(), &poolInfo, nullptr, &_command_pool) != VK_SUCCESS)
                {
                    LOG_AND_THROW(std::runtime_error("failed to create command pool!"));
                }
            } else {
                // Separate compute command pool.
                VkCommandPoolCreateInfo cmdPoolInfo = {};
                cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
                cmdPoolInfo.queueFamilyIndex = queueFamilyIndices.computeFamily.value();
                cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
                if (vkCreateCommandPool(_device.device(), &cmdPoolInfo, nullptr, &_command_pool) != VK_SUCCESS)
                {
                    LOG_AND_THROW(std::runtime_error("failed to create command pool!"));
                }
            }

            _command_buffers.resize(static_cast<size_t>(_images_count), VK_NULL_HANDLE);

            VkCommandBufferAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            allocInfo.commandPool = _command_pool;
            allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            allocInfo.commandBufferCount = (uint32_t)_command_buffers.size();

            if (vkAllocateCommandBuffers(_device.device(), &allocInfo, _command_buffers.data()) != VK_SUCCESS)
            {
                LOG_AND_THROW(std::runtime_error("failed to allocate command buffers!"));
            }
            std::clog << "Command buffers created" << std::endl;
        }

        PipelineType type() const {
            return _type;
        }

        VkCommandBuffer command_buffer(ImageIndex index) const
        {
            assert(static_cast<size_t>(index) < _command_buffers.size());
            return _command_buffers[static_cast<size_t>(index)];
        }

        VkCommandBuffer &command_buffer(ImageIndex index)
        {
            assert(static_cast<size_t>(index) < _command_buffers.size());
            return _command_buffers[static_cast<size_t>(index)];
        }

        VkCommandPool graphics_command_pool() const
        {
            assert(_type == PipelineType::Graphics);
            return _command_pool;
        }

        VkCommandPool compute_command_pool() const {
            assert(_type == PipelineType::Compute);
            return _command_pool;
        }

        VkCommandBuffer reset_record_graphics_command_buffer(VkFramebuffer frame_buffer,
                                                             VkExtent2D swap_chain_extent,
                                                             std::map<PipelineType, std::unique_ptr<Pipeline>> &pipelines,
                                                             const VertexBuffer &vertex_buffer,
                                                             DescriptorsManager &descriptors,
                                                             ImageIndex image_index,
                                                             VkClearValue background,
                                                             const RenderPass &render_pass);

        VkCommandBuffer reset_record_compute_command_buffer(std::map<PipelineType, std::unique_ptr<Pipeline>> &pipelines,
                                                 DescriptorsManager &descriptors,
                                                 ImageIndex image_index);

        void prepare_to_trace_barrier(ImageIndex current_image, VkImage image);
        void dispatch_raytrace(std::map<PipelineType, std::unique_ptr<Pipeline>> &pipelines,
                               DescriptorsManager &descriptors,
                               ImageIndex image_index);
        void prepare_to_present_barrier(ImageIndex image_index, VkImage image);
        void end_command_buffer(ImageIndex image_index);

    private:
        CommandBuffers(const CommandBuffers &) = delete;
        CommandBuffers &operator=(const CommandBuffers &) = delete;

        const Device &_device;
        const ImagesCount _images_count;
        const PipelineType _type;

        VkCommandPool _command_pool = VK_NULL_HANDLE;
        std::vector<VkCommandBuffer> _command_buffers;
    };

} // namespace
