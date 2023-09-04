#pragma once

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

#include "device.h"
#include "graphical_environment.h"
#include "ray_tracing_pipeline.h"
#include "validation.h"

namespace VulkanImpl {

// Top level class to setup the Vulkan.
class GraphicalEnvironment : public ::GraphicalEnvironment {
public:
    GraphicalEnvironment() {}
    ~GraphicalEnvironment() override {
        std::cerr << "Tearing down" << std::endl;
        _validation.reset();
        vkDestroySurfaceKHR(_instance, _surface, nullptr);
        vkDestroyInstance(_instance, 0);
        glfwDestroyWindow(_window);
        glfwTerminate();
    }

    void enable_validation();

    void init() override;

    void load_shader(const std::string& file, VkShaderStageFlagBits stage);

    void load_preconfigured_shapes() override {
        load_shader("../../build/assets/shaders/triangle.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
        load_shader("../../build/assets/shaders/triangle.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
    }

    // Separate init that requires the shaders to be loaded.
    void init_pipeline();

    void dump_device_info() const;

private:
    void window_init();
    void surface_init();

    VkInstance _instance = VK_NULL_HANDLE;
    GLFWwindow* _window = nullptr;
    VkSurfaceKHR _surface;
    std::unique_ptr<RayTracingPipeline> _pipeline;
    Device _device;
    std::vector<VkPipelineShaderStageCreateInfo> _loaded_shaders;
    std::unique_ptr<Validation> _validation;
};

}  // namespace
