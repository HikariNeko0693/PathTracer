#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <memory>
#include <string>
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

struct render_options
{
    int image_width = 400;
    int image_height = 225;
    int samples_per_pixel = 100;
    int max_depth = 50;
    int thread_count = static_cast<int>(std::thread::hardware_concurrency());
    unsigned int seed = 20260727;
    bool use_bvh = true;
    bool write_output = true;
    std::string output_path = "output/render.png";
};

struct benchmark_result
{
    double scene_build_ms = 0.0;
    double bvh_build_ms = 0.0;
    double render_ms = 0.0;
    double image_write_ms = 0.0;
    double total_ms = 0.0;
    size_t object_count = 0;
};

using clock_type = std::chrono::steady_clock;

static double elapsed_ms(clock_type::time_point begin, clock_type::time_point end)
{
    return std::chrono::duration<double, std::milli>(end - begin).count();
}

static int read_int_arg(int argc, char** argv, int& index, const char* name)
{
    if (index + 1 >= argc)
    {
        std::cerr << "Missing value for " << name << std::endl;
        std::exit(1);
    }

    return std::atoi(argv[++index]);
}

static unsigned int read_uint_arg(int argc, char** argv, int& index, const char* name)
{
    if (index + 1 >= argc)
    {
        std::cerr << "Missing value for " << name << std::endl;
        std::exit(1);
    }

    return static_cast<unsigned int>(std::strtoul(argv[++index], nullptr, 10));
}

static render_options parse_options(int argc, char** argv)
{
    render_options options;

    for (int i = 1; i < argc; ++i)
    {
        std::string arg = argv[i];

        if (arg == "--width")
            options.image_width = read_int_arg(argc, argv, i, "--width");
        else if (arg == "--height")
            options.image_height = read_int_arg(argc, argv, i, "--height");
        else if (arg == "--spp")
            options.samples_per_pixel = read_int_arg(argc, argv, i, "--spp");
        else if (arg == "--depth")
            options.max_depth = read_int_arg(argc, argv, i, "--depth");
        else if (arg == "--threads")
            options.thread_count = read_int_arg(argc, argv, i, "--threads");
        else if (arg == "--seed")
            options.seed = read_uint_arg(argc, argv, i, "--seed");
        else if (arg == "--no-bvh")
            options.use_bvh = false;
        else if (arg == "--no-output")
            options.write_output = false;
        else if (arg == "--output")
        {
            if (i + 1 >= argc)
            {
                std::cerr << "Missing value for --output" << std::endl;
                std::exit(1);
            }
            options.output_path = argv[++i];
        }
        else if (arg == "--help")
        {
            std::cout
                << "Usage: PathTracer.exe [options]\n"
                << "  --width N       Image width, default 400\n"
                << "  --height N      Image height, default 225\n"
                << "  --spp N         Samples per pixel, default 100\n"
                << "  --depth N       Max recursion depth, default 50\n"
                << "  --threads N     Worker threads, default hardware_concurrency\n"
                << "  --seed N        Fixed random seed, default 20260727\n"
                << "  --no-bvh        Render with linear scene traversal\n"
                << "  --no-output     Skip PNG writing\n"
                << "  --output PATH   PNG output path\n";
            std::exit(0);
        }
        else
        {
            std::cerr << "Unknown option: " << arg << std::endl;
            std::exit(1);
        }
    }

    options.image_width = std::max(1, options.image_width);
    options.image_height = std::max(1, options.image_height);
    options.samples_per_pixel = std::max(1, options.samples_per_pixel);
    options.max_depth = std::max(1, options.max_depth);
    options.thread_count = std::max(1, options.thread_count);
    options.thread_count = std::min(options.thread_count, options.image_height);

    return options;
}

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
            return attenuation * ray_color(scattered, world, depth - 1);

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
    int thread_id,
    int start_y,
    int end_y,
    const render_options& options,
    const camera& cam,
    const hittable& world,
    std::vector<vec3>& framebuffer
)
{
    set_random_seed(options.seed + static_cast<unsigned int>(thread_id * 1009));

    for (int j = start_y; j < end_y; j++)
    {
        for (int i = 0; i < options.image_width; i++)
        {
            vec3 pixel_color(0, 0, 0);

            for (int s = 0; s < options.samples_per_pixel; s++)
            {
                double u = (i + random_double()) / (options.image_width - 1);
                double v = (j + random_double()) / (options.image_height - 1);

                ray r = cam.get_ray(u, v);
                pixel_color += ray_color(r, world, options.max_depth);
            }

            framebuffer[j * options.image_width + i] = pixel_color;
        }
    }
}

static benchmark_result render(const render_options& options)
{
    benchmark_result result;
    auto total_begin = clock_type::now();

    set_random_seed(options.seed);

    auto scene_begin = clock_type::now();
    auto world = random_scene();
    auto scene_end = clock_type::now();
    result.scene_build_ms = elapsed_ms(scene_begin, scene_end);
    result.object_count = world.objects.size();

    std::unique_ptr<bvh_node> bvh;
    const hittable* render_world = &world;

    if (options.use_bvh)
    {
        auto bvh_begin = clock_type::now();
        bvh = std::make_unique<bvh_node>(world.objects, 0, world.objects.size());
        auto bvh_end = clock_type::now();
        result.bvh_build_ms = elapsed_ms(bvh_begin, bvh_end);
        render_world = bvh.get();
    }

    const double aspect_ratio = double(options.image_width) / options.image_height;

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

    std::vector<vec3> framebuffer(options.image_width * options.image_height);
    std::vector<std::thread> threads;

    auto render_begin = clock_type::now();

    for (int t = 0; t < options.thread_count; t++)
    {
        int start = t * options.image_height / options.thread_count;
        int end = (t + 1) * options.image_height / options.thread_count;

        threads.emplace_back(
            render_section,
            t,
            start,
            end,
            std::cref(options),
            std::cref(cam),
            std::cref(*render_world),
            std::ref(framebuffer)
        );
    }

    for (auto& th : threads)
        th.join();

    auto render_end = clock_type::now();
    result.render_ms = elapsed_ms(render_begin, render_end);

    if (options.write_output)
    {
        auto write_begin = clock_type::now();
        std::vector<unsigned char> image(options.image_width * options.image_height * 3);

        for (int j = 0; j < options.image_height; j++)
        {
            for (int i = 0; i < options.image_width; i++)
            {
                vec3 pixel = framebuffer[(options.image_height - 1 - j) * options.image_width + i];

                double scale = 1.0 / options.samples_per_pixel;

                double r = sqrt(pixel.x() * scale);
                double g = sqrt(pixel.y() * scale);
                double b = sqrt(pixel.z() * scale);

                int ir = static_cast<int>(256 * std::clamp(r, 0.0, 0.999));
                int ig = static_cast<int>(256 * std::clamp(g, 0.0, 0.999));
                int ib = static_cast<int>(256 * std::clamp(b, 0.0, 0.999));

                int index = (j * options.image_width + i) * 3;

                image[index + 0] = static_cast<unsigned char>(ir);
                image[index + 1] = static_cast<unsigned char>(ig);
                image[index + 2] = static_cast<unsigned char>(ib);
            }
        }

        int ok = stbi_write_png(
            options.output_path.c_str(),
            options.image_width,
            options.image_height,
            3,
            image.data(),
            options.image_width * 3
        );

        auto write_end = clock_type::now();
        result.image_write_ms = elapsed_ms(write_begin, write_end);

        if (!ok)
        {
            std::cerr << "Failed to write image: " << options.output_path << std::endl;
            std::exit(1);
        }
    }

    auto total_end = clock_type::now();
    result.total_ms = elapsed_ms(total_begin, total_end);

    return result;
}

static void print_result(const render_options& options, const benchmark_result& result)
{
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "Benchmark" << std::endl;
    std::cout << "Objects: " << result.object_count << std::endl;
    std::cout << "Resolution: " << options.image_width << "x" << options.image_height << std::endl;
    std::cout << "SPP: " << options.samples_per_pixel << std::endl;
    std::cout << "Max depth: " << options.max_depth << std::endl;
    std::cout << "Threads: " << options.thread_count << std::endl;
    std::cout << "Seed: " << options.seed << std::endl;
    std::cout << "Traversal: " << (options.use_bvh ? "BVH" : "Linear") << std::endl;
    std::cout << "Scene build: " << result.scene_build_ms << " ms" << std::endl;
    std::cout << "BVH build: " << result.bvh_build_ms << " ms" << std::endl;
    std::cout << "Render: " << result.render_ms << " ms" << std::endl;
    std::cout << "Image write: " << result.image_write_ms << " ms" << std::endl;
    std::cout << "Total: " << result.total_ms << " ms" << std::endl;
}

int main(int argc, char** argv)
{
    render_options options = parse_options(argc, argv);
    benchmark_result result = render(options);
    print_result(options, result);
    return 0;
}