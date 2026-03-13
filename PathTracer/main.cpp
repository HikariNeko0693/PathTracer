#include <iostream>
#include <fstream>
#include <memory>

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

hittable_list random_scene()
{
    hittable_list world;

    auto ground_material = std::make_shared<lambertian>(vec3(0.5, 0.5, 0.5));
    world.add(std::make_shared<sphere>(vec3(0, -1000, 0), 1000, ground_material));

    for (int a = -11; a < 11; a++)
    {
        for (int b = -11; b < 11; b++)
        {
            double choose_mat = random_double();

            vec3 center(
                a + 0.9 * random_double(),
                0.2,
                b + 0.9 * random_double()
            );

            if ((center - vec3(4, 0.2, 0)).length() > 0.9)
            {
                std::shared_ptr<material> sphere_material;

                if (choose_mat < 0.8)
                {
                    // diffuse
                    vec3 albedo = vec3(
                        random_double(),
                        random_double(),
                        random_double()
                    ) * vec3(
                        random_double(),
                        random_double(),
                        random_double()
                    );

                    sphere_material = std::make_shared<lambertian>(albedo);

                    world.add(std::make_shared<sphere>(center, 0.2, sphere_material));
                }
                else if (choose_mat < 0.95)
                {
                    // metal
                    vec3 albedo = vec3(
                        random_double() * 0.5 + 0.5,
                        random_double() * 0.5 + 0.5,
                        random_double() * 0.5 + 0.5
                    );

                    double fuzz = random_double() * 0.5;

                    sphere_material = std::make_shared<metal>(albedo, fuzz);

                    world.add(std::make_shared<sphere>(center, 0.2, sphere_material));
                }
                else
                {
                    // glass
                    sphere_material = std::make_shared<dielectric>(1.5);

                    world.add(std::make_shared<sphere>(center, 0.2, sphere_material));
                }
            }
        }
    }

    auto material1 = std::make_shared<dielectric>(1.5);
    world.add(std::make_shared<sphere>(vec3(0, 1, 0), 1.0, material1));

    auto material2 = std::make_shared<lambertian>(vec3(0.4, 0.2, 0.1));
    world.add(std::make_shared<sphere>(vec3(-4, 1, 0), 1.0, material2));

    auto material3 = std::make_shared<metal>(vec3(0.7, 0.6, 0.5), 0.0);
    world.add(std::make_shared<sphere>(vec3(4, 1, 0), 1.0, material3));

    return world;
}

int main()
{
    std::cout << "Start..." << std::endl;
    // Image
    const int image_width = 400;
    const int image_height = 225;

    const double aspect_ratio = double(image_width) / image_height;

    const int samples_per_pixel = 100;
    const int max_depth = 50;

    std::ofstream out("output/image.ppm");

    out << "P3\n" << image_width << " " << image_height << "\n255\n";

    lambertian ground(vec3(0.8, 0.8, 0.0));
    lambertian center(vec3(0.7, 0.3, 0.3));
    metal metal_ball(vec3(0.8, 0.8, 0.8), 0.0);
    // metal metal_ball(vec3(0.8, 0.8, 0.8), 0.3);  //Ä¥É°½ðÊô
    dielectric glass(1.5);

    auto world = random_scene();

    // Camera
    vec3 lookfrom(13, 2, 3);
    vec3 lookat(0, 0, 0);
    vec3 vup(0, 1, 0);

    double vfov = 90; // 120£º¹ã½Ç
    double aperture = 0.1;
    double focus_dist = 10;

    camera cam(
        vfov,
        aspect_ratio,
        lookfrom,
        lookat,
        vup,
        aperture,
        focus_dist
    );

    for (int j = image_height - 1;j >= 0;--j)
    {
        for (int i = 0;i < image_width;i++)
        {
            vec3 pixel_color(0, 0, 0);

            for (int s = 0;s < samples_per_pixel;s++)
            {
                double u = (i + random_double()) / (image_width - 1);
                double v = (j + random_double()) / (image_height - 1);

                ray r = cam.get_ray(u, v);

                pixel_color += ray_color(r, world, max_depth);
            }

            write_color(out, pixel_color, samples_per_pixel);
        }
    }
    std::cout << "End." << std::endl;
}