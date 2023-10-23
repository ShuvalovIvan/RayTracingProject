#include "vulkan/graphical_environment_vulkan.h"

#include <iostream>
#include <gtest/gtest.h>

using namespace RayTracingProject;

class RayTracingFixture : public ::testing::Test {
protected:
    std::vector<Sphere> default_spheres = {
        {{-0.5, -0.5, -0.5}, 0.1, {0.1f, 0.2f, 0.2f, 1.0f}},
        {{-0.5, -0.5,  0.5}, 0.1, {0.1f, 0.2f, 0.2f, 1.0f}}
    };

    void SetUp() override {
        _scene.enable_validation();
        _scene.load_preconfigured_shapes();
        _scene.add_texture("../../build/assets/textures/statue.jpg");
        _scene.init();
        _scene.dump_device_info();
        _scene.add_spheres(default_spheres);
        std::clog << "Test suite initialized" << std::endl;
    }

    void TearDown() override {}

    VulkanImpl::GraphicalEnvironment _scene;
};

TEST_F(RayTracingFixture, BasicVulkan) {
    _scene.start_interactive_loop(std::chrono::milliseconds(3000));
}
