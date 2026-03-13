#pragma once
#include "ray.h"
#include "vec3.h"
#include <cmath>

const double pi = 3.1415926535897932385;

class camera
{
public:

    vec3 origin;
    vec3 lower_left_corner;
    vec3 horizontal;
    vec3 vertical;

    vec3 u, v, w;
    double lens_radius;

    camera(
        double vfov,        // vertical field-of-view
        double aspect_ratio,
        vec3 lookfrom,  // 相机位置
        vec3 lookat,    // 观察目标
        vec3 vup,    // 上方向
        double aperture,
        double focus_dist
    )
    {
        // 角度 → 弧度
        double theta = vfov * pi / 180.0;

        // viewport高度
        double h = tan(theta / 2);

        double viewport_height = 2.0 * h;
        double viewport_width = aspect_ratio * viewport_height;

        // 相机坐标系
        vec3 w = unit_vector(lookfrom - lookat);
        vec3 u = unit_vector(cross(vup, w));
        vec3 v = cross(w, u);

        origin = lookfrom;

        horizontal = focus_dist * viewport_width * u;
        vertical = focus_dist * viewport_height * v;

        lower_left_corner =
            origin
            - horizontal / 2
            - vertical / 2
            - focus_dist * w;

        lens_radius = aperture / 2;
    }

    // 光线不再从 origin 发射, 而是从镜头圆盘随机位置发射
    ray get_ray(double s, double t) const
    {
        vec3 rd = lens_radius * random_in_unit_disk();
        vec3 offset = u * rd.x() + v * rd.y();

        return ray(
            origin + offset,
            lower_left_corner + s * horizontal + t * vertical - origin - offset
        );
    }
};
