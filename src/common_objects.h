#pragma once

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

namespace RayTracingProject
{

struct GraphicalEnvironmentSettings {
    int max_frames_in_flight = 2;
    int max_images = 2;
    int width = 1024;
    int height = 768;
    int sphere_count = 20;
};

struct Sphere
{
    glm::vec3 center;
    float radius;
    glm::vec4 color;
};

} // namespace
