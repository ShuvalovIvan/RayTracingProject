#pragma once

#include "buffer_base.h"

#include "command_buffers.h"

namespace VulkanImpl
{

class ComputeImage : public BufferBase {
public:
    ComputeImage(Device &device, CommandBuffers &cmd_buffers, BindingKey key)
        : BufferBase(device), _cmd_buffers(cmd_buffers), _key(key) {}

    ~ComputeImage() {

    }

    VkImageView texture_image_view() const {
        return _texture_image_view;
    }

    void init() {
        auto width = _device.swap_chain_extent().width;
        auto height = _device.swap_chain_extent().height;
        auto format = VK_FORMAT_R8G8B8A8_UNORM;

        VkImageCreateInfo imageCreateInfo{};
        imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
        imageCreateInfo.format = format;
        imageCreateInfo.extent = {width, height, 1};
        imageCreateInfo.mipLevels = 1;
        imageCreateInfo.arrayLayers = 1;
        imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        // Image will be sampled in the fragment shader and used as storage target in the compute shader
        imageCreateInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
        imageCreateInfo.flags = 0;

        if (vkCreateImage(_device.device(), &imageCreateInfo, nullptr, &_texture_image) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create compute image!");
        }

        VkMemoryAllocateInfo memAllocInfo{};
        VkMemoryRequirements memReqs{};
        vkGetImageMemoryRequirements(_device.device(), _texture_image, &memReqs);

        memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        memAllocInfo.allocationSize = memReqs.size;
        memAllocInfo.memoryTypeIndex = findMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        if (vkAllocateMemory(_device.device(), &memAllocInfo, nullptr, &_texture_image_memory) != VK_SUCCESS ||
            vkBindImageMemory(_device.device(), _texture_image, _texture_image_memory, 0) != VK_SUCCESS)
        {
            LOG_AND_THROW(std::runtime_error("failed to allocate vertex buffer memory!"));
        }

        auto command_buffer = beginSingleTimeCommands(_cmd_buffers.compute_command_pool());

        VkImageSubresourceRange subresourceRange = {};
        subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        subresourceRange.baseMipLevel = 0;
        subresourceRange.levelCount = 1;
        subresourceRange.layerCount = 1;

        // Create an image barrier object
        VkImageMemoryBarrier imageMemoryBarrier = {};
        imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
        imageMemoryBarrier.image = _texture_image;
        imageMemoryBarrier.subresourceRange = subresourceRange;
        imageMemoryBarrier.srcAccessMask = 0;
        imageMemoryBarrier.dstAccessMask = 0;

        // Put barrier inside setup command buffer
        vkCmdPipelineBarrier(
            command_buffer,
            VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
            VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
            0,
            0, nullptr,
            0, nullptr,
            1, &imageMemoryBarrier);

        endSingleTimeCommands(command_buffer, _cmd_buffers.compute_command_pool(), _device.compute_queue());

        // Create sampler
        VkSamplerCreateInfo sampler{};
        sampler.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        sampler.magFilter = VK_FILTER_LINEAR;
        sampler.minFilter = VK_FILTER_LINEAR;
        sampler.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        sampler.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        sampler.addressModeV = sampler.addressModeU;
        sampler.addressModeW = sampler.addressModeU;
        sampler.mipLodBias = 0.0f;
        sampler.maxAnisotropy = 1.0f;
        sampler.compareOp = VK_COMPARE_OP_NEVER;
        sampler.minLod = 0.0f;
        sampler.maxLod = 0.0f;
        sampler.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
        if (vkCreateSampler(_device.device(), &sampler, nullptr, &_texture_sampler) != VK_SUCCESS)
        {
            LOG_AND_THROW(std::runtime_error("failed to create texture sampler!"));
        }

        // Create image view
        VkImageViewCreateInfo view{};
        view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        view.viewType = VK_IMAGE_VIEW_TYPE_2D;
        view.format = format;
        view.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
        view.image = _texture_image;
        if (vkCreateImageView(_device.device(), &view, nullptr, &_texture_image_view) != VK_SUCCESS)
        {
            LOG_AND_THROW(std::runtime_error("failed to create image view!"));
        }
    }

private:
    const BindingKey _key;
    const CommandBuffers& _cmd_buffers;
    VkImage _texture_image;
    VkDeviceMemory _texture_image_memory;
    VkImageView _texture_image_view;
    VkSampler _texture_sampler;
};

}  // namespace

