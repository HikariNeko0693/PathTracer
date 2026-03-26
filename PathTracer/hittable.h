#pragma once
#include "ray.h"
#include "aabb.h"

class material;

struct hit_record {
    vec3 p;
    vec3 normal;
    std::shared_ptr<material> mat_ptr;

    double t = 0.0;              // 命中点在光线参数方程中的 t 值
    bool front_face = true;      // 是否命中物体外表面

    void set_face_normal(const ray& r, const vec3& outward_normal)
    {
        // 如果光线方向和外法线点乘 < 0，说明命中的是外表面
        front_face = dot(r.direction(), outward_normal) < 0;

        // 保证 rec.normal 始终与入射光线方向相对
        normal = front_face ? outward_normal : -outward_normal;
    }
};

class hittable {
public:
    virtual bool hit(
        const ray& r,
        double t_min,
        double t_max,
        hit_record& rec) const = 0;

    virtual bool bounding_box(aabb& output_box) const = 0;
};