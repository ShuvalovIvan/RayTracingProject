#include "camera.h"
#include "camera_cpu.h"
#include "hittable_list.h"
#include "material.h"
#include "sphere.h"

#include <gtest/gtest.h>

class RayTracingFixture : public ::testing::Test {
protected:
    void SetUp() override {
        cam.aspect_ratio      = 16.0 / 9.0;
        cam.image_width       = 400;
        cam.samples_per_pixel = 30;
        cam.max_depth         = 50;

        cam.vfov     = 20;
        cam.lookfrom = point3(13,2,3);
        cam.lookat   = point3(0,0,0);
        cam.vup      = vec3(0,1,0);

        cam.defocus_angle = 0.6;
        cam.focus_dist    = 10.0;
    }

    void add_sphere() {
        auto ground_material = make_shared<lambertian>(color(0.5, 0.5, 0.5));
        world.add(make_shared<sphere>(point3(0,-1000,0), 1000, ground_material));
    }

    hittable_list world;
    CPUImpl::Camera cam;
};

TEST_F(RayTracingFixture, PixelMatch) {
    add_sphere();

    cam.initialize();
    auto image_size = cam.image_size();
    auto center = std::make_pair(image_size.first / 2, image_size.second / 2);
    ray r = cam.get_ray(center.first, center.second);
    color ray_color = cam.ray_color(r, cam.max_depth, world);

    EXPECT_TRUE(ray_color.similar_to(color(0.253, 0.3518, 0.5))) << ray_color;
}

TEST_F(RayTracingFixture, Tmp) {
}
