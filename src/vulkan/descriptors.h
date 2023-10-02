#pragma once

#include <vulkan/vulkan.h>

#include "device.h"
#include "descriptor_set_layout.h"
#include "uniform_buffers.h"

namespace VulkanImpl
{

class Descriptors {
public:
    Descriptors(const Device &device, int max_frames) : _device(device), _max_frames(max_frames) {}

    ~Descriptors() {
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

    void init(const DescriptorSetLayout &descriptor_set_layout, const UniformBuffers &uniform_buffers)
    {
        VkDescriptorPoolSize poolSize{};
        poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSize.descriptorCount = static_cast<uint32_t>(_max_frames);

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = 1;
        poolInfo.pPoolSizes = &poolSize;
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

            VkWriteDescriptorSet descriptorWrite{};
            descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrite.dstSet = _descriptor_sets[i];
            descriptorWrite.dstBinding = 0;
            descriptorWrite.dstArrayElement = 0;
            descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrite.descriptorCount = 1;
            descriptorWrite.pBufferInfo = &bufferInfo;

            vkUpdateDescriptorSets(_device.device(), 1, &descriptorWrite, 0, nullptr);
        }
        std::clog << "Descriptors initialized" << std::endl;
    }

private:
    Descriptors(const Descriptors &) = delete;
    Descriptors &operator=(const Descriptors &) = delete;

    const Device &_device;
    const int _max_frames;
    VkDescriptorPool _descriptor_pool;
    std::vector<VkDescriptorSet> _descriptor_sets;
};

} // namespace
