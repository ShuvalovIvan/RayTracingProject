#include "vulkan/graphical_environment_vulkan.h"

#include <iostream>
#include <gtest/gtest.h>

class RayTracingFixture : public ::testing::Test {
protected:
    void SetUp() override {
        _scene.enable_validation();
        _scene.init();
        _scene.load_preconfigured_shapes();
        _scene.dump_device_info();
        _scene.init_pipeline();
    }

    void TearDown() override {}

    VulkanImpl::GraphicalEnvironment _scene;
};

TEST_F(RayTracingFixture, BasicVulkan) {
}
