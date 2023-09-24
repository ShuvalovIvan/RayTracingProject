#pragma once

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

#include "command_buffer.h"
#include "device.h"
#include "descriptor_set_layout.h"
#include "frame_buffer.h"
#include "graphical_environment.h"
#include "ray_tracing_pipeline.h"
#include "shader_modules.h"
#include "user_control.h"
#include "validation.h"
#include "vertex_buffer.h"
#include "uniform_buffers.h"

namespace VulkanImpl {

struct GraphicalEnvironmentSettings {
    int max_frames_in_flight = 2;
};

// Top level class to setup the Vulkan.
class GraphicalEnvironment : public ::GraphicalEnvironment {
public:
    GraphicalEnvironment(GraphicalEnvironmentSettings settings = {}) {}
    ~GraphicalEnvironment() override {
        std::cerr << "Tearing down" << std::endl;
        _pipeline.reset();
        _shader_modules.reset();
        _descriptor_set_layout.reset();
        _command_buffer.reset();
        _uniform_buffers.reset();
        vkDestroySemaphore(_device->device(), _render_finished_semaphore, nullptr);
        vkDestroySemaphore(_device->device(), _image_available_semaphore, nullptr);
        vkDestroyFence(_device->device(), _in_flight_fence, nullptr);
        _vertex_buffer.reset();
        _frame_buffers.clear();
        _device.reset();
        vkDestroySurfaceKHR(_instance, _surface, nullptr);
        _validation.reset();
        vkDestroyInstance(_instance, 0);
        std::cerr << "Instance deleted" << std::endl;
        glfwDestroyWindow(_window);
        std::cerr << "Window deleted" << std::endl;
        glfwTerminate();
        std::cerr << "Environment termninated" << std::endl;
    }

    void enable_validation();

    void init() override;

    void load_shader(const std::string& file, VkShaderStageFlagBits stage);

    void load_preconfigured_shapes() override {
        // load_shader("../../build/assets/shaders/triangle.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
        // load_shader("../../build/assets/shaders/triangle.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
        load_shader("../../build/assets/shaders/shader_buffers.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
        load_shader("../../build/assets/shaders/shader_buffers.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
    }

    // Separate init that requires the shaders to be loaded.
    void init_pipeline();

    void dump_device_info() const;

    void start_interactive_loop() override;

    void draw_frame();

private:
    void window_init();
    void surface_init();

    void frame_buffers_init();
    void synchronization_init();

    const GraphicalEnvironmentSettings _settings;
    VkInstance _instance = VK_NULL_HANDLE;
    GLFWwindow* _window = nullptr;
    VkSurfaceKHR _surface;
    std::unique_ptr<RayTracingPipeline> _pipeline;
    std::unique_ptr<Device> _device;
    std::unique_ptr<ShaderModules> _shader_modules;
    std::vector<FrameBuffer> _frame_buffers;
    std::unique_ptr<VertexBuffer> _vertex_buffer;
    std::unique_ptr<CommandBuffer> _command_buffer;
    std::unique_ptr<UniformBuffers> _uniform_buffers;
    std::unique_ptr<Validation> _validation;
    std::unique_ptr <DescriptorSetLayout> _descriptor_set_layout;
    UserControl _user_control;

    VkSemaphore _image_available_semaphore;
    VkSemaphore _render_finished_semaphore;
    VkFence _in_flight_fence;
};

}  // namespace
