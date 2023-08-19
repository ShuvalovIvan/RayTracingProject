#include "camera.h"

namespace CPUImpl {
    class Camera : public camera {
    public:
        ~Camera() override = default;

        virtual color ray_color(const ray& r, int depth, const hittable& world) const {
            hit_record rec;

            // If we've exceeded the ray bounce limit, no more light is gathered.
            if (depth <= 0)
                return color(0,0,0);

            if (world.hit(r, interval(0.001, infinity), rec)) {
                ray scattered;
                color attenuation;
                if (rec.mat->scatter(r, rec, attenuation, scattered))
                    return attenuation * ray_color(scattered, depth-1, world);
                return color(0,0,0);
            }

            vec3 unit_direction = unit_vector(r.direction());
            auto a = 0.5*(unit_direction.y() + 1.0);
            return (1.0-a)*color(1.0, 1.0, 1.0) + a*color(0.5, 0.7, 1.0);
        }

    };

}  // namespace
