#pragma once

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "command_buffers.h"
#include "device.h"
#include "descriptor_set_layout.h"
#include "descriptors.h"
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
        std::clog << "Tearing down" << std::endl;
        for (auto f : _in_flight_fences) {
            vkWaitForFences(_device->device(), 1, &f, VK_TRUE, UINT64_MAX);
        }
        _frame_buffers.clear();
        _device->cleanup_swap_chain();
        _pipeline.reset();
        _shader_modules.reset();
        _descriptor_set_layout.reset();
        _descriptors.reset();
        _uniform_buffers.reset();
        for (int i = 0; i < _settings.max_frames_in_flight; ++i) {
            vkDestroySemaphore(_device->device(), _render_finished_semaphores[i], nullptr);
            vkDestroySemaphore(_device->device(), _image_available_semaphores[i], nullptr);
            vkDestroyFence(_device->device(), _in_flight_fences[i], nullptr);
        }
        _vertex_buffer.reset();

        _command_buffers.reset();
        _device.reset();
        vkDestroySurfaceKHR(_instance, _surface, nullptr);
        _validation.reset();
        vkDestroyInstance(_instance, 0);
        std::clog << "Instance deleted" << std::endl;
        glfwDestroyWindow(_window);
        std::clog << "Window deleted" << std::endl;
        glfwTerminate();
        std::clog << "Environment termninated" << std::endl;
    }

    void enable_validation();

    void init() override;

    void load_shader(const std::string& file, VkShaderStageFlagBits stage);

    void load_preconfigured_shapes() override {
        // load_shader("../../build/assets/shaders/triangle.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
        // load_shader("../../build/assets/shaders/triangle.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
        load_shader("../../build/assets/shaders/shader_buffers.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
        load_shader("../../build/assets/shaders/shader_buffers.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
        std::clog << "Shaders loaded" << std::endl;
    }

    // Separate init that requires the shaders to be loaded.
    void init_pipeline();

    void dump_device_info() const;

    void start_interactive_loop() override;

    void draw_frame();

    void recreate_swap_chain();

private:
    void window_init();
    void surface_init();

    void frame_buffers_init();
    void synchronization_init();

    void update_uniform_buffer(uint32_t currentImage);

    const GraphicalEnvironmentSettings _settings;
    uint32_t _current_frame = 0;
    bool _framebuffer_resized = false;

    VkInstance _instance = VK_NULL_HANDLE;
    GLFWwindow* _window = nullptr;
    VkSurfaceKHR _surface;
    std::unique_ptr<RayTracingPipeline> _pipeline;
    std::unique_ptr<Device> _device;
    std::unique_ptr<ShaderModules> _shader_modules;
    std::vector<FrameBuffer> _frame_buffers;
    std::unique_ptr<VertexBuffer> _vertex_buffer;
    std::unique_ptr<CommandBuffers> _command_buffers;
    std::unique_ptr<UniformBuffers> _uniform_buffers;
    std::unique_ptr<Validation> _validation;
    std::unique_ptr<DescriptorSetLayout> _descriptor_set_layout;
    std::unique_ptr<Descriptors> _descriptors;
    UserControl _user_control;

    std::vector<VkSemaphore> _image_available_semaphores;
    std::vector<VkSemaphore> _render_finished_semaphores;
    std::vector<VkFence> _in_flight_fences;
};

}  // namespace
