#include "vulkan/graphical_environment_vulkan.h"

#include <iostream>
#include <gtest/gtest.h>

class RayTracingFixture : public ::testing::Test {
protected:
    void SetUp() override {
        _scene.enable_validation();
        _scene.init();
        _scene.load_preconfigured_shapes();
        _scene.add_texture("../../build/assets/textures/statue.jpg");
        _scene.dump_device_info();
        _scene.init_pipeline();
        std::clog << "Test suite initialized" << std::endl;
    }

    void TearDown() override {}

    VulkanImpl::GraphicalEnvironment _scene;
};

TEST_F(RayTracingFixture, BasicVulkan) {
    for (int i = 0; i < 100; ++i) {
        _scene.draw_frame();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}
