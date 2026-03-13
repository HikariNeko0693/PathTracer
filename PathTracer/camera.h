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

    camera(
        double vfov,        // vertical field-of-view
        double aspect_ratio,
        vec3 lookfrom,  // 相机位置
        vec3 lookat,    // 观察目标
        vec3 vup    // 上方向
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

        horizontal = viewport_width * u;
        vertical = viewport_height * v;

        lower_left_corner =
            origin
            - horizontal / 2
            - vertical / 2
            - w;
    }

    ray get_ray(double s, double t) const
    {
        return ray(
            origin,
            lower_left_corner + s * horizontal + t * vertical - origin
        );
    }
};
