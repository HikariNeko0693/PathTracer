#pragma once
#include "hittable.h"
#include "aabb.h"
#include <memory>

class sphere : public hittable
{
public:
    vec3 center;                         // 球心
    double radius = 0;                  // 半径
    std::shared_ptr<material> mat_ptr;  // 材质

    sphere() {}

    sphere(
        vec3 c,
        double r,
        std::shared_ptr<material> m
    )
        : center(c), radius(r), mat_ptr(m) {
    }

    virtual bool hit(
        const ray& r,
        double t_min,
        double t_max,
        hit_record& rec
    ) const override
    {
        // 光线起点到球心的向量
        vec3 oc = r.origin() - center;

        // 将光线方程代入球方程
        auto a = r.direction().length_squared();
        auto half_b = dot(oc, r.direction());
        auto c = oc.length_squared() - radius * radius;

        // 小于 0 说明没有交点
        auto discriminant = half_b * half_b - a * c;

        if (discriminant < 0) return false;

        auto sqrtd = sqrt(discriminant);

        // 较近的根
        auto root = (-half_b - sqrtd) / a;

        // 如果较近的根不合法，再尝试较远的根
        if (root < t_min || root > t_max)
        {
            root = (-half_b + sqrtd) / a;

            if (root < t_min || root > t_max)
                return false;
        }

        // 记录命中参数 t
        rec.t = root;

        // 根据 t 求实际命中点
        rec.p = r.at(rec.t);

        // 球面法线 = (命中点 - 球心) / 半径
        vec3 outward_normal = (rec.p - center) / radius;

        // 修正法线方向，并记录是否命中外表面
        rec.set_face_normal(r, outward_normal);

        // 保存材质指针，供后续散射使用
        rec.mat_ptr = mat_ptr;

        return true;
    }

    virtual bool bounding_box(aabb& output_box) const override
    {
        // 球体的 AABB：以球心为中心，向三个轴方向扩 radius
        output_box = aabb(
            center - vec3(radius, radius, radius),
            center + vec3(radius, radius, radius)
        );

        return true;
    }
};