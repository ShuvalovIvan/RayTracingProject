#include "shader_loader.h"

#include <assert.h>

#include "graphical_environment.h"

namespace VulkanImpl
{

VkPipelineShaderStageCreateInfo ShaderLoader::load_shader_module()
{
    std::ifstream file(_file, std::ios::ate | std::ios::binary);

    if (!file.is_open())
    {
        LOG_AND_THROW(std::runtime_error("failed to open file '" + _file + "'"));
    }

    const auto fileSize = static_cast<size_t>(file.tellg());
    assert(fileSize > 1);
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);
    if (!file) {
        LOG_AND_THROW(std::runtime_error("failed to read '" + file.gcount()));
    }
    file.close();
    assert(!buffer.empty());

    VkShaderModuleCreateInfo module_create_info{};
    module_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    module_create_info.codeSize = buffer.size();
    module_create_info.pCode = reinterpret_cast<const uint32_t *>(buffer.data());
    module_create_info.flags = 0;
    module_create_info.pNext = nullptr;

    if (VK_SUCCESS != vkCreateShaderModule(_device.device(), &module_create_info, nullptr, &_shader_module))
    {
        LOG_AND_THROW(std::runtime_error("create shader module"));
    }

    VkPipelineShaderStageCreateInfo stage_create_info{};
    stage_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stage_create_info.stage = _stage;
    stage_create_info.module = _shader_module;
    stage_create_info.pName = "main";
    stage_create_info.pNext = nullptr;
    stage_create_info.flags = 0;
    stage_create_info.pSpecializationInfo = nullptr;

    return stage_create_info;
}

} // namespace
