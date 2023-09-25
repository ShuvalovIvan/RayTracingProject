#pragma once

#include <vector>

#include <vulkan/vulkan.h>

#include "vulkan/device.h"
#include "vulkan/descriptor_set_layout.h"
#include "vulkan/shader_modules.h"

namespace VulkanImpl {

class RayTracingPipeline {
public:
    RayTracingPipeline(const Device& device) : _device(device) {}
    ~RayTracingPipeline() {
        vkDestroyPipelineLayout(_device.device(), _pipeline_layout, nullptr);
        vkDestroyRenderPass(_device.device(), _render_pass, nullptr);
    }

    void init(ShaderModules &shader_modules, DescriptorSetLayout &descriptor_set_layout);

    VkRenderPass render_pass() const {
        return _render_pass;
    }

    VkPipeline pipeline() const {
        return _pipeline;
    }

    VkPipelineLayout pipeline_layout() const {
        return _pipeline_layout;
    }

private:
    void init_render_pass();
    void init_graphics_pipeline(ShaderModules &shader_modules, DescriptorSetLayout &descriptor_set_layout);

    const Device& _device;
	VkPipelineLayout _pipeline_layout = VK_NULL_HANDLE;
    VkRenderPass _render_pass = VK_NULL_HANDLE;
    VkPipeline _pipeline = VK_NULL_HANDLE;
};

}  // namespace
