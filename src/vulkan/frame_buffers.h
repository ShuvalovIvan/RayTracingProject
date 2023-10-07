#pragma once

#include <vector>

#include <vulkan/vulkan.h>

namespace VulkanImpl
{

    class FrameBuffers
    {
    public:
        FrameBuffers(const Device& device, size_t size) : _device(device), _size(size) {}

        ~FrameBuffers()
        {
            for (auto f : _frame_buffers) {
                if (f != VK_NULL_HANDLE) {
                    vkDestroyFramebuffer(_device.device(), f, nullptr);
                }
            }
            std::clog << "Frame buffer destroyed" << std::endl;
        }

        void init(VkRenderPass render_pass, const std::vector<VkImageView>& swap_chain_image_views)
        {
            assert(_device.swap_chain_extent().width > 10);
            assert(_device.swap_chain_extent().height > 10);
            assert(swap_chain_image_views.size() == _size);
            _frame_buffers.resize(_size);

            for (int i = 0; i < _size; ++i) {
                VkImageView attachments[] = {swap_chain_image_views[i]};
                VkFramebufferCreateInfo framebufferInfo{};
                framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
                framebufferInfo.renderPass = render_pass;
                framebufferInfo.attachmentCount = 1;
                framebufferInfo.pAttachments = attachments;
                framebufferInfo.width = _device.swap_chain_extent().width;
                framebufferInfo.height = _device.swap_chain_extent().height;
                framebufferInfo.layers = 1;

                if (vkCreateFramebuffer(_device.device(), &framebufferInfo, nullptr, &_frame_buffers[i]) != VK_SUCCESS)
                {
                    LOG_AND_THROW(std::runtime_error("failed to create framebuffer!"));
                }
            }
        }

        std::vector<VkFramebuffer> frame_buffers()
        {
            return _frame_buffers;
        }

        size_t size() const {
            return _frame_buffers.size();
        }

    private:
        FrameBuffers(const FrameBuffers&) = delete;
        FrameBuffers &operator=(const FrameBuffers&) = delete;

        const Device &_device;
        const size_t _size;
        std::vector<VkFramebuffer> _frame_buffers;
    };

} // namespace
