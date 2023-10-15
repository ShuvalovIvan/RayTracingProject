#pragma once

#include <vulkan/vulkan.h>

#include <vector>

#include "vulkan/shader_loader.h"

namespace VulkanImpl
{

    class ShaderModules
    {
    public:
        ShaderModules() {}

        ~ShaderModules() = default;

        void add_shader_module(const std::string& file, VkShaderStageFlagBits stage) {
            _loaders.emplace_back(file, stage);
        }

        std::vector<VkPipelineShaderStageCreateInfo> load_all_stages(const Device &device, PipelineType type) {
            std::vector<VkPipelineShaderStageCreateInfo> result;

            for (auto& loader : _loaders) {
                if (type == PipelineType::Compute && loader.stage() == VK_SHADER_STAGE_COMPUTE_BIT) {
                    result.push_back(loader.load_shader_module(&device));
                } else if (type == PipelineType::Graphics && loader.stage() != VK_SHADER_STAGE_COMPUTE_BIT) {
                    result.push_back(loader.load_shader_module(&device));
                }
            }
            return result;
        }

    private:
        ShaderModules(const ShaderModules &) = delete;
        ShaderModules &operator=(const ShaderModules &) = delete;

        std::vector<ShaderLoader> _loaders;
    };

} // namespace
