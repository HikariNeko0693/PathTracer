#include <iostream>
#include <fstream>
#include <memory>
#include <thread>
#include <vector>

#include "vec3.h"
#include "ray.h"
#include "color.h"
#include "sphere.h"
#include "hittable_list.h"
#include "lambertian.h"
#include "metal.h"
#include "dielectric.h"
#include "camera.h"
#include "aabb.h"
#include "bvh.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

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

void render_section(
    int start_y,
    int end_y,
    int image_width,
    int image_height,
    int samples_per_pixel,
    int max_depth,
    camera& cam,
    const hittable& world,
    std::vector<vec3>& framebuffer
)
{
    for (int j = start_y; j < end_y; j++)
    {
        for (int i = 0; i < image_width; i++)
        {
            vec3 pixel_color(0, 0, 0);

            for (int s = 0; s < samples_per_pixel; s++)
            {
                double u = (i + random_double()) / (image_width - 1);
                double v = (j + random_double()) / (image_height - 1);

                ray r = cam.get_ray(u, v);
                pixel_color += ray_color(r, world, max_depth);
            }

            framebuffer[j * image_width + i] = pixel_color;
        }
    }
}

int main()
{
    std::cout << "Start..." << std::endl;

    const int image_width = 400;
    const int image_height = 225;
    const double aspect_ratio = double(image_width) / image_height;

    const int samples_per_pixel = 100;
    const int max_depth = 50;

    std::vector<vec3> framebuffer(image_width * image_height);

    auto world = random_scene();
    bvh_node bvh(world.objects, 0, world.objects.size());

    vec3 lookfrom(13, 2, 3);
    vec3 lookat(0, 0, 0);
    vec3 vup(0, 1, 0);

    double vfov = 90;
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

    int thread_count = std::thread::hardware_concurrency();
    std::vector<std::thread> threads;

    int rows_per_thread = image_height / thread_count;

    for (int t = 0; t < thread_count; t++)
    {
        int start = t * rows_per_thread;

        int end = (t == thread_count - 1)
            ? image_height
            : start + rows_per_thread;

        threads.emplace_back(
            render_section,
            start,
            end,
            image_width,
            image_height,
            samples_per_pixel,
            max_depth,
            std::ref(cam),
            std::ref(bvh),
            std::ref(framebuffer)
        );
    }

    for (auto& th : threads)
    {
        th.join();
    }

    std::vector<unsigned char> image(image_width * image_height * 3);

    for (int j = 0; j < image_height; j++)
    {
        for (int i = 0; i < image_width; i++)
        {
            vec3 pixel = framebuffer[(image_height - 1 - j) * image_width + i];

            double scale = 1.0 / samples_per_pixel;

            double r = sqrt(pixel.x() * scale);
            double g = sqrt(pixel.y() * scale);
            double b = sqrt(pixel.z() * scale);

            int ir = static_cast<int>(256 * std::clamp(r, 0.0, 0.999));
            int ig = static_cast<int>(256 * std::clamp(g, 0.0, 0.999));
            int ib = static_cast<int>(256 * std::clamp(b, 0.0, 0.999));

            int index = (j * image_width + i) * 3;

            image[index + 0] = ir;
            image[index + 1] = ig;
            image[index + 2] = ib;
        }
    }

    stbi_write_png(
        "output/render.png",
        image_width,
        image_height,
        3,
        image.data(),
        image_width * 3
    );

    std::cout << "End." << std::endl;
}