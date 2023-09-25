#pragma once

#include <vulkan/vulkan.h>

namespace VulkanImpl
{

class DescriptorSetLayout {
public:
    DescriptorSetLayout(const Device& device) : _device(device) {}

    ~DescriptorSetLayout() {
        vkDestroyDescriptorSetLayout(_device.device(), _descriptor_set_layout, nullptr);
    }

    void init()
    {
        VkDescriptorSetLayoutBinding uboLayoutBinding{};
        uboLayoutBinding.binding = 0;
        uboLayoutBinding.descriptorCount = 1;
        uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboLayoutBinding.pImmutableSamplers = nullptr;
        uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = 1;
        layoutInfo.pBindings = &uboLayoutBinding;

        if (vkCreateDescriptorSetLayout(_device.device(), &layoutInfo, nullptr, &_descriptor_set_layout) != VK_SUCCESS)
        {
            LOG_AND_THROW(std::runtime_error("failed to create descriptor set layout!"));
        }
    }

    VkDescriptorSetLayout& descriptor_set_layout() {
        return _descriptor_set_layout;
    }

private:
    const Device &_device;
    VkDescriptorSetLayout _descriptor_set_layout;
};

} // namespace
