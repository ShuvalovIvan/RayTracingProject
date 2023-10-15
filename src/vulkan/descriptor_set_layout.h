#pragma once

#include <array>

#include <vulkan/vulkan.h>

namespace VulkanImpl
{

class DescriptorSetLayout {
public:
    DescriptorSetLayout(const Device &device, PipelineType type) : _device(device), _type(type) {}

    virtual ~DescriptorSetLayout() {
        vkDestroyDescriptorSetLayout(_device.device(), _descriptor_set_layout, nullptr);
    }

    virtual void init()
    {
        assert(_type == PipelineType::Graphics);
        VkDescriptorSetLayoutBinding uboLayoutBinding{};
        uboLayoutBinding.binding = 0;
        uboLayoutBinding.descriptorCount = 1;
        uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboLayoutBinding.pImmutableSamplers = nullptr;
        uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

        VkDescriptorSetLayoutBinding samplerLayoutBinding{};
        samplerLayoutBinding.binding = 1;
        samplerLayoutBinding.descriptorCount = 1;
        samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        samplerLayoutBinding.pImmutableSamplers = nullptr;
        samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        std::array<VkDescriptorSetLayoutBinding, 2> bindings = {uboLayoutBinding, samplerLayoutBinding};
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

    virtual PipelineType type() const {
        return PipelineType::Graphics;
    }

    VkDescriptorSetLayout descriptor_set_layout() const
    {
        return _descriptor_set_layout;
    }

    VkDescriptorSetLayout &descriptor_set_layout()
    {
        return _descriptor_set_layout;
    }

protected:
    DescriptorSetLayout(const DescriptorSetLayout &) = delete;
    DescriptorSetLayout &operator=(const DescriptorSetLayout &) = delete;

    const Device &_device;
    PipelineType _type;
    VkDescriptorSetLayout _descriptor_set_layout;
};

class ComputeDescriptorSetLayout : public DescriptorSetLayout
{
public:
    ComputeDescriptorSetLayout(const Device &device, PipelineType type)
        : DescriptorSetLayout(device, type) {}

    PipelineType type() const override
    {
        return PipelineType::Compute;
    }

    void init() override
    {
        assert(_type == PipelineType::Compute);
        std::array<VkDescriptorSetLayoutBinding, 3> layoutBindings{};
        layoutBindings[0].binding = 0;
        layoutBindings[0].descriptorCount = 1;
        layoutBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        layoutBindings[0].pImmutableSamplers = nullptr;
        layoutBindings[0].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

        layoutBindings[1].binding = 1;
        layoutBindings[1].descriptorCount = 1;
        layoutBindings[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        layoutBindings[1].pImmutableSamplers = nullptr;
        layoutBindings[1].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

        layoutBindings[2].binding = 2;
        layoutBindings[2].descriptorCount = 1;
        layoutBindings[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        layoutBindings[2].pImmutableSamplers = nullptr;
        layoutBindings[2].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = 3;
        layoutInfo.pBindings = layoutBindings.data();

        if (vkCreateDescriptorSetLayout(_device.device(), &layoutInfo, nullptr, &_descriptor_set_layout) != VK_SUCCESS)
        {
            LOG_AND_THROW(std::runtime_error("failed to create descriptor set layout!"));
        }
        std::clog << "Descriptor set layout initialized" << std::endl;
    }
};

} // namespace
