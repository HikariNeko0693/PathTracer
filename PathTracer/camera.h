#pragma once
#include "ray.h"
#include "vec3.h"
#include <cmath>

const double pi = 3.1415926535897932385;

class camera
{
public:
    vec3 origin;             // 相机位置
    vec3 lower_left_corner;  // 视口左下角在世界空间中的位置
    vec3 horizontal;         // 视口水平边向量
    vec3 vertical;           // 视口竖直边向量

    vec3 u, v, w;            // 相机局部坐标系：右、上、后
    double lens_radius;      // 镜头半径，用于景深采样

    camera(
        double vfov,         // 垂直视场角
        double aspect_ratio,
        vec3 lookfrom,       // 相机位置
        vec3 lookat,         // 观察目标
        vec3 vup,            // 世界空间参考上方向
        double aperture,     // 光圈大小
        double focus_dist    // 对焦距离
    )
    {
        // 视场角从角度转成弧度
        double theta = vfov * pi / 180.0;

        // 根据 tan(vfov / 2) 求出视口半高
        double h = tan(theta / 2);

        // 得到视口宽高
        double viewport_height = 2.0 * h;
        double viewport_width = aspect_ratio * viewport_height;

        // 构建相机局部坐标系
        w = unit_vector(lookfrom - lookat);   // 相机朝后方向
        u = unit_vector(cross(vup, w));       // 相机右方向
        v = cross(w, u);                      // 相机上方向

        origin = lookfrom;

        // 视口在世界空间中的水平和竖直跨度
        horizontal = focus_dist * viewport_width * u;
        vertical = focus_dist * viewport_height * v;

        // 计算视口左下角的位置
        lower_left_corner =
            origin
            - horizontal / 2
            - vertical / 2
            - focus_dist * w;

        // 光圈转为镜头半径
        lens_radius = aperture / 2;
    }

    ray get_ray(double s, double t) const
    {
        // 在单位圆盘内随机采样一个点，再乘镜头半径
        vec3 rd = lens_radius * random_in_unit_disk();

        // 将二维镜头采样偏移映射到世界空间
        vec3 offset = u * rd.x() + v * rd.y();

        return ray(
            origin + offset,  // 光线起点改在镜头圆盘内随机偏移
            lower_left_corner + s * horizontal + t * vertical - origin - offset
        );
    }
};