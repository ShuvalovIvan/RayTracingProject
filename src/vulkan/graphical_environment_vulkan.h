#pragma once

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

#include "device.h"
#include "graphical_environment.h"
#include "ray_tracing_pipeline.h"

namespace VulkanImpl {

// Top level class to setup the Vulkan.
class GraphicalEnvironment : public ::GraphicalEnvironment {
public:
    GraphicalEnvironment() {}
    ~GraphicalEnvironment() override {
        std::cerr << "Tearing down" << std::endl;
        vkDestroyInstance(_instance, 0);
        glfwDestroyWindow(_window);
        glfwTerminate();
    }

    void init() override;

    void load_shader(const std::string& file, VkShaderStageFlagBits stage);

    void load_preconfigured_shapes() override {
        load_shader("../../build/assets/shaders/triangle.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
    }

private:
    void window_init();
    void surface_init();

    VkInstance _instance = VK_NULL_HANDLE;
    GLFWwindow* _window = nullptr;
    VkSurfaceKHR _surface;
    RayTracingPipeline _pipeline;
    Device _device;
};

}  // namespace
