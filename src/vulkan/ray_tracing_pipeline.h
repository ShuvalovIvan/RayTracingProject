#pragma once

#include <vector>

#include <vulkan/vulkan.h>

#include "vulkan/device.h"
#include "vulkan/shader_modules.h"

namespace VulkanImpl {

class RayTracingPipeline {
public:
    RayTracingPipeline(const Device& device) : _device(device) {}
    ~RayTracingPipeline() {
        vkDestroyPipelineLayout(_device.device(), _pipeline_layout, nullptr);
        vkDestroyRenderPass(_device.device(), _render_pass, nullptr);
    }

    void init(ShaderModules& shader_modules);

    VkRenderPass render_pass() const {
        return _render_pass;
    }

private:
    void init_render_pass();
    void init_graphics_pipeline(ShaderModules &shader_modules);

    const Device& _device;
	VkPipelineLayout _pipeline_layout = VK_NULL_HANDLE;
    VkRenderPass _render_pass = VK_NULL_HANDLE;
    VkPipeline _pipeline = VK_NULL_HANDLE;
};

}  // namespace
