#pragma once

#include <vector>

#include <vulkan/vulkan.h>

#include "vulkan/device.h"
#include "vulkan/descriptor_set_layout.h"
#include "vulkan/render_pass.h"
#include "vulkan/shader_modules.h"

namespace VulkanImpl {

class Pipeline {
public:
    VkPipeline pipeline() const
    {
        return _pipeline;
    }

    VkPipelineLayout pipeline_layout() const
    {
        return _pipeline_layout;
    }

protected:
    Pipeline(const Device& device) : _device(device) {}
    virtual ~Pipeline() {}

    virtual void init(ShaderModules &shader_modules, DescriptorSetLayout &descriptor_set_layout, const RenderPass& render_pass) = 0;

    const Device &_device;
    VkPipelineLayout _pipeline_layout = VK_NULL_HANDLE;
    VkPipeline _pipeline = VK_NULL_HANDLE;
};

class GraphicsPipeline : public Pipeline
{
public:
    GraphicsPipeline(const Device& device) : Pipeline(device) {}
    ~GraphicsPipeline() override {
        vkDestroyPipeline(_device.device(), _pipeline, nullptr);
        vkDestroyPipelineLayout(_device.device(), _pipeline_layout, nullptr);
    }

    void init(ShaderModules &shader_modules, DescriptorSetLayout &descriptor_set_layout, const RenderPass &render_pass) override;

private:
    GraphicsPipeline(const GraphicsPipeline &) = delete;
    GraphicsPipeline &operator=(const GraphicsPipeline &) = delete;

    void init_pipeline_layout(DescriptorSetLayout &descriptor_set_layout);
    void init_graphics_pipeline(ShaderModules &shader_modules, DescriptorSetLayout &descriptor_set_layout, const RenderPass &render_pass);
};

}  // namespace
