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
    ShaderLoader(const std::string &file, VkShaderStageFlagBits stage)
        : _file(file), _stage(stage) {}

    ShaderLoader(ShaderLoader&& other) : _file(other._file),
        _stage(other._stage), _shader_module(other._shader_module) {
        other._shader_module = VK_NULL_HANDLE;
    }

    VkShaderStageFlagBits stage() const {
        return _stage;
    }

    ~ShaderLoader()
    {
        if (_shader_module != nullptr && _device != nullptr)
        {
            vkDestroyShaderModule(_device->device(), _shader_module, nullptr);
        }
    }

    VkPipelineShaderStageCreateInfo load_shader_module(const Device *device);

private:
    ShaderLoader(const ShaderLoader &) = delete;
    ShaderLoader &operator=(const ShaderLoader &) = delete;

    const std::string _file;
    const VkShaderStageFlagBits _stage;
    const Device *_device = nullptr;
    VkShaderModule _shader_module = nullptr;
};

}  // namespace
