#pragma once

#include <vulkan/vulkan.h>

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "device.h"

namespace VulkanImpl {

class ShaderLoader {
public:
    ShaderLoader(const std::string& file, Device &device) : _file(file), _device(device) {}

    VkPipelineShaderStageCreateInfo load_shader_module(VkShaderStageFlagBits stage);

private:
    const std::string _file;
    const Device &_device;
};


}  // namespace
