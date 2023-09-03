#include "vulkan/graphical_environment_vulkan.h"

#include <iostream>
#include <gtest/gtest.h>

class RayTracingFixture : public ::testing::Test {
protected:
    void SetUp() override {
        _scene.init();
        _scene.load_preconfigured_shapes();
        _scene.init_pipeline();
    }

    void TearDown() override {}

    VulkanImpl::GraphicalEnvironment _scene;
};

TEST_F(RayTracingFixture, BasicVulkan) {
}
