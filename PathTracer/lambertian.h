#pragma once
#include "material.h"

class lambertian : public material
{
public:
    vec3 albedo;  // 材质本身的颜色（反照率）

    lambertian(const vec3& a) : albedo(a) {}

    virtual bool scatter(
        const ray& r_in,
        const hit_record& rec,
        vec3& attenuation,
        ray& scattered
    ) const override
    {
        // 漫反射散射方向
        vec3 scatter_direction = rec.normal + random_unit_vector();

        // 如果随机结果刚好把方向抵消掉，就直接退化为沿法线方向散射，防止出现接近零向量的情况
        if (scatter_direction.near_zero())
            scatter_direction = rec.normal;

        // 新光线从交点 rec.p 出发，朝 scatter_direction 方向传播
        scattered = ray(rec.p, scatter_direction);

        // 漫反射材质的颜色衰减由 albedo 决定，表面会吸收一部分颜色，只保留自身颜色对应的能量
        attenuation = albedo;

        return true;  // 漫反射总是继续散射
    }
};