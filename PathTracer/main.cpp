#include <iostream>
#include <fstream>

#include "vec3.h"
#include "ray.h"
#include "color.h"
#include "sphere.h"
#include "hittable_list.h"
#include "lambertian.h"
#include "metal.h"
#include "dielectric.h"
#include "camera.h"

vec3 ray_color(const ray& r, const hittable& world, int depth)
{
    if (depth <= 0)
        return vec3(0, 0, 0);

    hit_record rec;

    if (world.hit(r, 0.001, 1e8, rec))
    {
        ray scattered;
        vec3 attenuation;

        if (rec.mat_ptr->scatter(r, rec, attenuation, scattered))
        {
            return attenuation * ray_color(scattered, world, depth - 1);
        }

        return vec3(0, 0, 0);
    }

    vec3 unit_direction = unit_vector(r.direction());

    double t = 0.5 * (unit_direction.y() + 1.0);

    return vec3(1, 1, 1) * (1.0 - t) + vec3(0.5, 0.7, 1) * t;
}

int main()
{
    // Image
    const int image_width = 400;
    const int image_height = 225;

    const double aspect_ratio = double(image_width) / image_height;

    const int samples_per_pixel = 50;

    std::ofstream out("output/image.ppm");

    out << "P3\n" << image_width << " " << image_height << "\n255\n";

    lambertian ground(vec3(0.8, 0.8, 0.0));
    lambertian center(vec3(0.7, 0.3, 0.3));
    metal metal_ball(vec3(0.8, 0.8, 0.8), 0.0);
    // metal metal_ball(vec3(0.8, 0.8, 0.8), 0.3);  //Ä¥É°½ðÊô
    dielectric glass(1.5);

    sphere s1(vec3(0, -100.5, -1), 100, &ground);
    sphere s2(vec3(0, 0, -1), 0.5, &center);
    sphere s3(vec3(1, 0, -1), 0.5, &metal_ball);
    sphere s4(vec3(-1, 0, -1), 0.5, &glass);
    sphere s5(vec3(-1, 0, -1), -0.45, &glass);

    hittable_list world;
    world.add(&s1);
    world.add(&s2);
    world.add(&s3);
    world.add(&s4);
    world.add(&s5);

    // Camera
    double viewport_height = 2.0;
    double viewport_width = aspect_ratio * viewport_height;
    double focal_length = 1.0;

    vec3 origin(0, 0, 0);

    vec3 horizontal(viewport_width, 0, 0);
    vec3 vertical(0, viewport_height, 0);

    vec3 lower_left_corner =
        origin
        - horizontal / 2
        - vertical / 2
        - vec3(0, 0, focal_length);

    for (int j = image_height - 1;j >= 0;--j)
    {
        for (int i = 0;i < image_width;i++)
        {
            vec3 pixel_color(0, 0, 0);

            for (int s = 0;s < samples_per_pixel;s++)
            {
                double u = (i + random_double()) / (image_width - 1);
                double v = (j + random_double()) / (image_height - 1);

                vec3 direction =
                    lower_left_corner +
                    horizontal * u +
                    vertical * v -
                    origin;

                ray r(origin, direction);

                pixel_color = pixel_color + ray_color(r, world, 10);
            }

            write_color(out, pixel_color, samples_per_pixel);
        }
    }
}