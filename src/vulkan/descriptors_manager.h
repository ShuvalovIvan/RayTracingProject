#pragma once

#include <vulkan/vulkan.h>

#include <array>

#include "vulkan_common_objects.h"
#include "device.h"

namespace VulkanImpl
{

class Texture;
class UniformBuffers;

struct Binding {
    BindingSequence binding;
    VkDescriptorType descriptor_type;
    BindingType binding_type;
    BindingKey key;
    VkShaderStageFlags shader_flags;
    BindingsMaxCount max_count;
};

inline constexpr Binding s_bindings[] =
{
    { BindingSequence::COMMON_UBO, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, BindingType::Buffer, BindingKey::CommonUBO,
        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_COMPUTE_BIT, BindingsMaxCount{1} },

    { BindingSequence::TEXTURE_IMAGE_SAMPLER, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, BindingType::Image, BindingKey::PrimaryTexture,
        VK_SHADER_STAGE_FRAGMENT_BIT, BindingsMaxCount{1} }
};


class DescriptorsManager {
public:
    DescriptorsManager(const Device &device, const RayTracingProject::GraphicalEnvironmentSettings &settings)
        : _device(device), _settings(settings) {}

    ~DescriptorsManager();

    void init(const std::vector<std::unique_ptr<Texture>> &textures, const UniformBuffers &uniformBuffers);

    VkDescriptorSet descriptor(FrameIndex frame_index) const
    {
        return _descriptor_sets[static_cast<uint32_t>(frame_index)];
    }

    VkDescriptorSet& descriptor(FrameIndex frame_index)
    {
        return _descriptor_sets[static_cast<uint32_t>(frame_index)];
    }

    VkDescriptorSetLayout descriptor_set_layout() const {
        return _descriptor_set_layout;
    }

private:
    void init_pool();
    void init_layout();
    void init_descriptors(const std::vector<std::unique_ptr<Texture>> &textures, const UniformBuffers &uniformBuffers);

    const Device &_device;
    const RayTracingProject::GraphicalEnvironmentSettings _settings;

    VkDescriptorPool _descriptor_pool = VK_NULL_HANDLE;
    VkDescriptorSetLayout _descriptor_set_layout = VK_NULL_HANDLE;

    std::vector<VkDescriptorSet> _descriptor_sets;
};

}  // namespace
