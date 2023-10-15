#pragma once

#include <map>
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "command_buffers.h"
#include "device.h"
#include "descriptor_set_layout.h"
#include "descriptors.h"
#include "frame_buffers.h"
#include "graphical_environment.h"
#include "ray_tracing_pipeline.h"
#include "render_pass.h"
#include "shader_modules.h"
#include "texture.h"
#include "user_control.h"
#include "validation.h"
#include "vertex_buffer.h"
#include "uniform_buffers.h"

namespace VulkanImpl {

struct GraphicalEnvironmentSettings {
    int max_frames_in_flight = 1;
    int max_images = 2;
    int width = 1024;
    int height = 768;
};

// Top level class to setup the Vulkan.
class GraphicalEnvironment : public ::GraphicalEnvironment {
public:
    GraphicalEnvironment(GraphicalEnvironmentSettings settings = {}) {
        _shader_modules = std::make_unique<ShaderModules>();
    }

    ~GraphicalEnvironment() override {
        std::clog << "Tearing down" << std::endl;
        for (auto f : _in_flight_fences) {
            vkWaitForFences(_device->device(), 1, &f, VK_TRUE, UINT64_MAX);
        }
        _frame_buffers.reset();
        _device->cleanup_swap_chain();
        _pipelines.clear();
        _render_pass.reset();
        _shader_modules.reset();
        _descriptor_set_layouts.clear();
        _descriptors.clear();
        _uniform_buffers.reset();
        for (int i = 0; i < _settings.max_frames_in_flight; ++i) {
            vkDestroySemaphore(_device->device(), _render_finished_semaphores[i], nullptr);
            vkDestroySemaphore(_device->device(), _image_available_semaphores[i], nullptr);
            vkDestroyFence(_device->device(), _in_flight_fences[i], nullptr);
        }
        _vertex_buffer.reset();
        _command_buffers.clear();
        _textures.clear();

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

    void add_texture(const std::string& file) override {
        _texture_files.push_back(file);
    }

    void dump_device_info() const;

    void start_interactive_loop(int loops = 100, std::chrono::milliseconds sleep = std::chrono::milliseconds(0)) override;

    void draw_frame();

    void recreate_swap_chain();

    void framebuffer_resized() {
        _framebuffer_resized = true;
    }

private:
    GraphicalEnvironment(const GraphicalEnvironment &) = delete;
    GraphicalEnvironment &operator=(const GraphicalEnvironment &) = delete;

    void init_pipeline();

    void window_init();
    void surface_init();

    void frame_buffers_init();
    void synchronization_init();

    void update_uniform_buffer(uint32_t currentImage);
    void update_backgroung_color();

    const GraphicalEnvironmentSettings _settings;
    uint32_t _current_frame = 0;
    bool _framebuffer_resized = false;

    VkInstance _instance = VK_NULL_HANDLE;
    GLFWwindow* _window = nullptr;
    VkSurfaceKHR _surface;
    std::map<PipelineType, std::unique_ptr<GraphicsPipeline>> _pipelines;
    std::unique_ptr<Device> _device;
    std::unique_ptr<RenderPass> _render_pass;
    std::unique_ptr<ShaderModules> _shader_modules;
    std::unique_ptr<FrameBuffers> _frame_buffers;
    std::unique_ptr<VertexBuffer> _vertex_buffer;
    std::map<PipelineType, std::unique_ptr<CommandBuffers>> _command_buffers;
    std::unique_ptr<UniformBuffers> _uniform_buffers;
    std::unique_ptr<Validation> _validation;
    std::map<PipelineType, std::unique_ptr<DescriptorSetLayout>> _descriptor_set_layouts;
    std::map<PipelineType, std::unique_ptr<Descriptors>> _descriptors;

    std::vector<std::string> _texture_files;
    std::vector<std::unique_ptr<Texture>> _textures;

    UserControl _user_control;
    VkClearValue _background = {{{0.0f, 0.0f, 0.0f, 1.0f}}};

    std::vector<VkSemaphore> _image_available_semaphores;
    std::vector<VkSemaphore> _render_finished_semaphores;
    std::vector<VkFence> _in_flight_fences;
};

}  // namespace
