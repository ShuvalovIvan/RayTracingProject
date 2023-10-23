#pragma once

#include "descriptors_manager.h"

namespace VulkanImpl
{

using namespace RayTracingProject;

DescriptorsManager::~DescriptorsManager() {
    vkDestroyDescriptorPool(_device.device(), _descriptor_pool, nullptr);
    vkDestroyDescriptorSetLayout(_device.device(), _descriptor_set_layout, nullptr);
}

void DescriptorsManager::init() {
    init_pool();
    init_layout();
    init_descriptors();
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

void DescriptorsManager::init_descriptors() {
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

        for (int b = 0; b < bindings_size; ++b) {
            const Binding& binding = s_bindings[b];
            std::vector<VkDescriptorImageInfo> image_infos(bindings_size, VkDescriptorImageInfo{});
            std::vector<VkDescriptorBufferInfo> buffer_infos(bindings_size, VkDescriptorBufferInfo{});

            switch (binding.binding_type) {
                case BindingType::Image: {
                    VkDescriptorImageInfo imageInfo{};
                    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                    imageInfo.imageView = texture.texture_image_view();
                    imageInfo.sampler = texture.texture_sampler();
                }
                break;

                case BindingType::Buffer: {

                } break;

                case BindingType::Acceleration:
                    assert(false);
            }
        }

        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = uniform_buffers.uniform_buffer(i);
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(UniformBufferObject);

        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = texture.texture_image_view();
        imageInfo.sampler = texture.texture_sampler();

        std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = _descriptor_sets[i];
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo = &bufferInfo;

        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].dstSet = _descriptor_sets[i];
        descriptorWrites[1].dstBinding = 1;
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].pImageInfo = &imageInfo;

        vkUpdateDescriptorSets(_device.device(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
    }
    std::clog << "Descriptors initialized" << std::endl;
}

} // namespace
