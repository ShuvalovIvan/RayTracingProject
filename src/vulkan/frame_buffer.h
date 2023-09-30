#pragma once

#include <vulkan/vulkan.h>

namespace VulkanImpl
{

    class FrameBuffer
    {
    public:
        FrameBuffer(const Device& device, int image_index) : _device(device), _image_index(image_index) {}
        FrameBuffer(FrameBuffer&& other)
        : _device(other._device), _image_index(other._image_index), _frame_buffer(other._frame_buffer) {
            other._frame_buffer = VK_NULL_HANDLE;
        }

        ~FrameBuffer()
        {
            if (_frame_buffer != VK_NULL_HANDLE) {
                vkDestroyFramebuffer(_device.device(), _frame_buffer, nullptr);
            }
        }

        void init(VkRenderPass render_pass, VkImageView swap_chain_image_view)
        {
            VkImageView attachments[] = { swap_chain_image_view };

            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = render_pass;
            framebufferInfo.attachmentCount = 1;
            framebufferInfo.pAttachments = attachments;
            framebufferInfo.width = _device.swap_chain_extent().width;
            framebufferInfo.height = _device.swap_chain_extent().height;
            framebufferInfo.layers = 1;

            if (vkCreateFramebuffer(_device.device(), &framebufferInfo, nullptr, &_frame_buffer) != VK_SUCCESS)
            {
                LOG_AND_THROW(std::runtime_error("failed to create framebuffer!"));
            }
        }

        VkFramebuffer frame_buffer() {
            return _frame_buffer;
        }

        int image_index() const {
            return _image_index;
        }

    private:
        FrameBuffer(const FrameBuffer&) = delete;
        FrameBuffer &operator=(const FrameBuffer&) = delete;

        const Device &_device;
        const int _image_index;
        VkFramebuffer _frame_buffer = VK_NULL_HANDLE;
    };

} // namespace
