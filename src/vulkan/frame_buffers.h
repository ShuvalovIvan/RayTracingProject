#pragma once

#include <vector>

#include <vulkan/vulkan.h>

#include "render_pass.h"

namespace VulkanImpl
{

    class FrameBuffers
    {
    public:
        FrameBuffers(const Device &device, ImagesCount image_count) : _device(device), _image_count(image_count) {}

        ~FrameBuffers()
        {
            for (auto f : _frame_buffers) {
                if (f != VK_NULL_HANDLE) {
                    vkDestroyFramebuffer(_device.device(), f, nullptr);
                }
            }
            std::clog << "Frame buffer destroyed" << std::endl;
        }

        void init(const RenderPass& render_pass)
        {
            assert(_device.swap_chain_extent().width > 10);
            assert(_device.swap_chain_extent().height > 10);
            _frame_buffers.resize(static_cast<uint32_t>(_image_count));

            for (int i = 0; i < static_cast<uint32_t>(_image_count); ++i)
            {
                VkImageView attachments[] = { _device.swap_chain_image_view(ImageIndex(i)) };
                VkFramebufferCreateInfo framebufferInfo{};
                framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
                framebufferInfo.renderPass = render_pass.render_pass();
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
        const ImagesCount _image_count;
        std::vector<VkFramebuffer> _frame_buffers;
    };

} // namespace
