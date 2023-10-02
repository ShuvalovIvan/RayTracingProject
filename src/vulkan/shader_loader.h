#pragma once

#include <vulkan/vulkan.h>

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "vulkan/device.h"

namespace VulkanImpl {

class ShaderLoader {
public:
    ShaderLoader(const std::string &file, const Device &device, VkShaderStageFlagBits stage)
        : _file(file), _device(device), _stage(stage) {}

    ShaderLoader(ShaderLoader&& other) : _file(other._file), _device(other._device),
        _stage(other._stage), _shader_module(other._shader_module) {
        other._shader_module = VK_NULL_HANDLE;
    }

    ~ShaderLoader()
    {
        if (_shader_module != nullptr)
        {
            vkDestroyShaderModule(_device.device(), _shader_module, nullptr);
        }
    }

    VkPipelineShaderStageCreateInfo load_shader_module();

private:
    ShaderLoader(const ShaderLoader &) = delete;
    ShaderLoader &operator=(const ShaderLoader &) = delete;

    const std::string _file;
    const Device &_device;
    const VkShaderStageFlagBits _stage;
    VkShaderModule _shader_module = nullptr;
};

}  // namespace
