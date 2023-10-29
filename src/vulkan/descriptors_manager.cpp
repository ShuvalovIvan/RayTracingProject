#pragma once

#include "descriptors_manager.h"

#include "texture.h"
#include "uniform_buffers.h"

namespace VulkanImpl
{

using namespace RayTracingProject;

DescriptorsManager::~DescriptorsManager() {
    if (_descriptor_pool) {
        vkDestroyDescriptorPool(_device.device(), _descriptor_pool, nullptr);
        vkDestroyDescriptorSetLayout(_device.device(), _descriptor_set_layout, nullptr);
    }
}

void DescriptorsManager::init(const std::vector<std::unique_ptr<Texture>> &textures, const UniformBuffers &uniformBuffers)
{
    init_pool();
    init_layout();
    init_descriptors(textures, uniformBuffers);
}

void DescriptorsManager::init_pool()
{
    assert(_settings.max_frames_in_flight > 0);
    std::array<VkDescriptorPoolSize, sizeof(s_bindings) / sizeof(s_bindings[0])> poolSizes{};

    for (int i = 0; i < poolSizes.size(); ++i) {
        poolSizes[i].type = s_bindings[i].descriptor_type;
        poolSizes[i].descriptorCount = static_cast<uint32_t>(_settings.max_frames_in_flight);
    }

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = static_cast<uint32_t>(_settings.max_frames_in_flight);

    if (vkCreateDescriptorPool(_device.device(), &poolInfo, nullptr, &_descriptor_pool) != VK_SUCCESS)
    {
        LOG_AND_THROW(std::runtime_error("failed to create descriptor pool!"));
    }
}

void DescriptorsManager::init_layout() {
    std::array<VkDescriptorSetLayoutBinding, sizeof(s_bindings) / sizeof(s_bindings[0])> bindings;

    for (int i = 0; i < bindings.size(); ++i)
    {
        VkDescriptorSetLayoutBinding& b = bindings[i];
        b = {};
        b.binding = i;
        b.descriptorCount = static_cast<uint32_t>(s_bindings[i].max_count);  // When repetitive.
        b.descriptorType = s_bindings[i].descriptor_type;
        b.pImmutableSamplers = nullptr;
        b.stageFlags = s_bindings[i].shader_flags;
    }

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    if (vkCreateDescriptorSetLayout(_device.device(), &layoutInfo, nullptr, &_descriptor_set_layout) != VK_SUCCESS)
    {
        LOG_AND_THROW(std::runtime_error("failed to create descriptor set layout!"));
    }
    std::clog << "Descriptor set layout initialized" << std::endl;
}

void DescriptorsManager::init_descriptors(const std::vector<std::unique_ptr<Texture>> &textures, const UniformBuffers& uniform_buffers)
{
    std::vector<VkDescriptorSetLayout> layouts(_settings.max_frames_in_flight, _descriptor_set_layout);
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = _descriptor_pool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(_settings.max_frames_in_flight);
    allocInfo.pSetLayouts = layouts.data();

    _descriptor_sets.resize(_settings.max_frames_in_flight);
    if (vkAllocateDescriptorSets(_device.device(), &allocInfo, _descriptor_sets.data()) != VK_SUCCESS)
    {
        LOG_AND_THROW(std::runtime_error("failed to allocate descriptor sets!"));
    }

    for (size_t i = 0; i < _settings.max_frames_in_flight; i++)
    {
        VkDescriptorSet& descriptor_set = _descriptor_sets[i];
        constexpr auto bindings_size = sizeof(s_bindings) / sizeof(s_bindings[0]);
        std::array<VkWriteDescriptorSet, bindings_size> descriptorWrites{};
        std::vector<VkDescriptorImageInfo> image_infos;
        std::vector<VkDescriptorBufferInfo> buffer_infos;
        image_infos.reserve(bindings_size);
        buffer_infos.reserve(bindings_size);

        for (int b = 0, tex = 0; b < bindings_size; ++b) {
            const Binding& binding = s_bindings[b];
            VkWriteDescriptorSet& write = descriptorWrites[b];
            write = {};

            switch (binding.binding_type) {
                case BindingType::Image: {
                    image_infos.push_back(VkDescriptorImageInfo{});
                    VkDescriptorImageInfo &imageInfo = image_infos.back();
                    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                    imageInfo.imageView = textures[tex]->texture_image_view();
                    imageInfo.sampler = textures[tex++]->texture_sampler();

                    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    write.dstSet = _descriptor_sets[i];
                    write.dstBinding = static_cast<uint32_t>(binding.binding);
                    write.dstArrayElement = 0;
                    write.descriptorType = binding.descriptor_type;
                    write.descriptorCount = 1;
                    write.pImageInfo = &image_infos.back();
                }
                break;

                case BindingType::SwapChainImage:
                {
                    image_infos.push_back(VkDescriptorImageInfo{});
                    VkDescriptorImageInfo& imageInfo = image_infos.back();
                    imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
                    imageInfo.imageView = _device.swap_chain_image_views()[i];
                    imageInfo.sampler = nullptr;

                    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    write.dstSet = _descriptor_sets[i];
                    write.dstBinding = static_cast<uint32_t>(binding.binding);
                    write.dstArrayElement = 0;
                    write.descriptorType = binding.descriptor_type;
                    write.descriptorCount = 1;
                    write.pImageInfo = &image_infos.back();
                }
                break;

                case BindingType::Buffer: {
                    VkDescriptorBufferInfo bufferInfo{};
                    bufferInfo.buffer = uniform_buffers.uniform_buffer(FrameIndex(i), binding.key);
                    bufferInfo.offset = 0;
                    bufferInfo.range = uniform_buffers.uniform_buffer_size(FrameIndex(i), binding.key);
                    buffer_infos.push_back(bufferInfo);

                    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    write.dstSet = _descriptor_sets[i];
                    write.dstBinding = static_cast<uint32_t>(binding.binding);
                    write.dstArrayElement = 0;
                    write.descriptorType = binding.descriptor_type;
                    write.descriptorCount = 1;
                    write.pBufferInfo = &buffer_infos.back();
                }
                break;

                case BindingType::Acceleration:
                    assert(false);
            }
        }

        vkUpdateDescriptorSets(_device.device(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
    }
    std::clog << "Descriptors initialized" << std::endl;
}

} // namespace
