#pragma once

#include <vulkan/vulkan.h>

#include <vector>

#include "vulkan/shader_loader.h"

namespace VulkanImpl
{

    class ShaderModules
    {
    public:
        ShaderModules(Device &device) : _device(device) {}

        ~ShaderModules() = default;

        void add_shader_module(const std::string& file, VkShaderStageFlagBits stage) {
            _loaders.emplace_back(file, _device, stage);
        }

        std::vector<VkPipelineShaderStageCreateInfo> load_all_stages() {
            std::vector<VkPipelineShaderStageCreateInfo> result;

            for (auto& loader : _loaders) {
                result.push_back(loader.load_shader_module());
            }
            return result;
        }

    private:
        const Device &_device;
        std::vector<ShaderLoader> _loaders;
    };

} // namespace
