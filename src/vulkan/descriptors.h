#pragma once

#include <vulkan/vulkan.h>

#include "device.h"
#include "descriptor_set_layout.h"
#include "texture.h"
#include "uniform_buffers.h"

namespace VulkanImpl
{

class Descriptors {
public:
    Descriptors(const Device &device, int max_frames, PipelineType type)
    : _device(device), _max_frames(max_frames), _type(type) {}

    virtual ~Descriptors() {
        vkDestroyDescriptorPool(_device.device(), _descriptor_pool, nullptr);
    }

    VkDescriptorSet& descriptor(int frame_index)
    {
        assert(frame_index < _descriptor_sets.size());
        return _descriptor_sets[frame_index];
    }

    VkDescriptorSet descriptor(int frame_index) const
    {
        return _descriptor_sets[frame_index];
    }

    virtual void init(const DescriptorSetLayout &descriptor_set_layout, const UniformBuffers &uniform_buffers, const Texture& texture)
    {
        assert(_type == PipelineType::Graphics);
        assert(descriptor_set_layout.type() == PipelineType::Graphics);
        assert(_max_frames > 0);
        std::array<VkDescriptorPoolSize, 2> poolSizes{};
        poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSizes[0].descriptorCount = static_cast<uint32_t>(_max_frames);
        poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        poolSizes[1].descriptorCount = static_cast<uint32_t>(_max_frames);

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        poolInfo.pPoolSizes = poolSizes.data();
        poolInfo.maxSets = static_cast<uint32_t>(_max_frames);

        if (vkCreateDescriptorPool(_device.device(), &poolInfo, nullptr, &_descriptor_pool) != VK_SUCCESS)
        {
            LOG_AND_THROW(std::runtime_error("failed to create descriptor pool!"));
        }

        std::vector<VkDescriptorSetLayout> layouts(_max_frames, descriptor_set_layout.descriptor_set_layout());
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = _descriptor_pool;
        allocInfo.descriptorSetCount = static_cast<uint32_t>(_max_frames);
        allocInfo.pSetLayouts = layouts.data();

        _descriptor_sets.resize(_max_frames);
        if (vkAllocateDescriptorSets(_device.device(), &allocInfo, _descriptor_sets.data()) != VK_SUCCESS)
        {
            LOG_AND_THROW(std::runtime_error("failed to allocate descriptor sets!"));
        }

        for (size_t i = 0; i < _max_frames; i++)
        {
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

protected:
    Descriptors(const Descriptors &) = delete;
    Descriptors &operator=(const Descriptors &) = delete;

    const Device &_device;
    const int _max_frames;
    PipelineType _type;
    VkDescriptorPool _descriptor_pool;
    std::vector<VkDescriptorSet> _descriptor_sets;
};

class ComputeDescriptors : public Descriptors
{
public:
    ComputeDescriptors(const Device &device, int max_frames, PipelineType type)
    : Descriptors(device, max_frames, type) {}

    ~ComputeDescriptors() override {}

    void init(const DescriptorSetLayout &descriptor_set_layout, const UniformBuffers &uniform_buffers, const Texture &texture) override
    {
        assert(_type == PipelineType::Compute);
        assert(descriptor_set_layout.type() == PipelineType::Compute);
        std::array<VkDescriptorPoolSize, 2> poolSizes{};
        poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSizes[0].descriptorCount = static_cast<uint32_t>(_max_frames);
        poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        poolSizes[1].descriptorCount = static_cast<uint32_t>(_max_frames);

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        poolInfo.pPoolSizes = poolSizes.data();
        poolInfo.maxSets = static_cast<uint32_t>(_max_frames);

        if (vkCreateDescriptorPool(_device.device(), &poolInfo, nullptr, &_descriptor_pool) != VK_SUCCESS)
        {
            LOG_AND_THROW(std::runtime_error("failed to create descriptor pool!"));
        }

        std::vector<VkDescriptorSetLayout> layouts(_max_frames, descriptor_set_layout.descriptor_set_layout());
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = _descriptor_pool;
        allocInfo.descriptorSetCount = static_cast<uint32_t>(_max_frames);
        allocInfo.pSetLayouts = layouts.data();

        _descriptor_sets.resize(_max_frames);
        if (vkAllocateDescriptorSets(_device.device(), &allocInfo, _descriptor_sets.data()) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to allocate descriptor sets!");
        }

        for (size_t i = 0; i < _max_frames; i++)
        {
/*
            VkDescriptorBufferInfo uniformBufferInfo{};
            uniformBufferInfo.buffer = uniformBuffers[i];
            uniformBufferInfo.offset = 0;
            uniformBufferInfo.range = sizeof(UniformBufferObject);

            std::array<VkWriteDescriptorSet, 3> descriptorWrites{};
            descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[0].dstSet = _descriptor_sets[i];
            descriptorWrites[0].dstBinding = 0;
            descriptorWrites[0].dstArrayElement = 0;
            descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrites[0].descriptorCount = 1;
            descriptorWrites[0].pBufferInfo = &uniformBufferInfo;

            VkDescriptorBufferInfo storageBufferInfoLastFrame{};
            storageBufferInfoLastFrame.buffer = shaderStorageBuffers[(i - 1) % MAX_FRAMES_IN_FLIGHT];
            storageBufferInfoLastFrame.offset = 0;
            storageBufferInfoLastFrame.range = sizeof(Particle) * PARTICLE_COUNT;

            descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[1].dstSet = _descriptor_sets[i];
            descriptorWrites[1].dstBinding = 1;
            descriptorWrites[1].dstArrayElement = 0;
            descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            descriptorWrites[1].descriptorCount = 1;
            descriptorWrites[1].pBufferInfo = &storageBufferInfoLastFrame;

            VkDescriptorBufferInfo storageBufferInfoCurrentFrame{};
            storageBufferInfoCurrentFrame.buffer = shaderStorageBuffers[i];
            storageBufferInfoCurrentFrame.offset = 0;
            storageBufferInfoCurrentFrame.range = sizeof(Particle) * PARTICLE_COUNT;

            descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[2].dstSet = _descriptor_sets[i];
            descriptorWrites[2].dstBinding = 2;
            descriptorWrites[2].dstArrayElement = 0;
            descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            descriptorWrites[2].descriptorCount = 1;
            descriptorWrites[2].pBufferInfo = &storageBufferInfoCurrentFrame;

            vkUpdateDescriptorSets(device, 3, descriptorWrites.data(), 0, nullptr);
*/
        }
    }
};

} // namespace
