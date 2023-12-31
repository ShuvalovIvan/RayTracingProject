#pragma once

#include <map>
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "command_buffers.h"
#include "common_objects.h"
#include "compute_image.h"
#include "data_buffer.h"
#include "device.h"
#include "descriptors_manager.h"
#include "frame.h"
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


// Top level class to setup the Vulkan.
class GraphicalEnvironment : public ::RayTracingProject::GraphicalEnvironment
{
public:
    GraphicalEnvironment(RayTracingProject::GraphicalEnvironmentSettings settings = {})
    {
        _shader_modules = std::make_unique<ShaderModules>();
    }

    ~GraphicalEnvironment() override {
        std::clog << "Tearing down" << std::endl;
        _frames.clear();
        _frame_buffers.reset();
        _spheres_buffer.reset();
        _device->cleanup_swap_chain();
        _pipelines.clear();
        _render_pass.reset();
        _shader_modules.reset();
        _descriptors_manager.reset();
        _uniform_buffers.reset();
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
        load_shader("../../build/assets/shaders/ray_tracing.comp.spv", VK_SHADER_STAGE_COMPUTE_BIT);
        std::clog << "Shaders loaded" << std::endl;
    }

    void add_texture(const std::string& file) override {
        _texture_files.push_back(file);
    }

    void add_spheres(const std::vector<RayTracingProject::Sphere> &spheres) override;

    void dump_device_info() const;

    void start_interactive_loop(std::chrono::milliseconds duration = std::chrono::seconds(3)) override;

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
    void frames_init();

    ImageIndex draw_frame_computational();

    void draw_frame_graphical(ImageIndex imageIndex);

    void update_uniform_buffer(FrameIndex current_frame);
    void update_backgroung_color();

    void image_memory_barrier(
        const VkCommandBuffer commandBuffer,
        const VkImage image,
        const VkImageSubresourceRange subresourceRange,
        const VkAccessFlags srcAccessMask,
        const VkAccessFlags dstAccessMask,
        const VkImageLayout oldLayout,
        const VkImageLayout newLayout);

    const RayTracingProject::GraphicalEnvironmentSettings _settings;
    uint32_t _current_frame = 0;
    bool _framebuffer_resized = false;

    VkInstance _instance = VK_NULL_HANDLE;
    GLFWwindow* _window = nullptr;
    VkSurfaceKHR _surface;
    std::map<PipelineType, std::unique_ptr<Pipeline>> _pipelines;
    std::unique_ptr<Device> _device;
    std::unique_ptr<RenderPass> _render_pass;
    std::unique_ptr<ShaderModules> _shader_modules;
    std::unique_ptr<FrameBuffers> _frame_buffers;
    std::unique_ptr<VertexBuffer> _vertex_buffer;
    std::unique_ptr<DataBuffer<RayTracingProject::Sphere, 200>> _spheres_buffer;
    std::map<PipelineType, std::unique_ptr<CommandBuffers>> _command_buffers;
    std::unique_ptr<UniformBuffers> _uniform_buffers;
    std::unique_ptr<Validation> _validation;
    std::unique_ptr<DescriptorsManager> _descriptors_manager;

    std::vector<std::string> _texture_files;
    std::vector<std::unique_ptr<Texture>> _textures;
    std::unique_ptr<ComputeImage> _compute_image;

    std::vector<std::unique_ptr<Frame>> _frames;

    UserControl _user_control;
    VkClearValue _background = {{{0.0f, 0.0f, 0.0f, 0.5f}}};
};

}  // namespace
