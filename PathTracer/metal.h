#pragma once
#include "material.h"

class metal : public material
{
public:
    vec3 albedo;   // 金属表面的基础颜色
    double fuzz;   // 粗糙度

    metal(const vec3& a, double f)
        : albedo(a), fuzz(f < 1 ? f : 1) {   // fuzz 限制在 [0,1]
    }

    virtual bool scatter(
        const ray& r_in,
        const hit_record& rec,
        vec3& attenuation,
        ray& scattered
    ) const override
    {
        // 先将入射方向单位化，再根据法线计算镜面反射方向
        vec3 reflected =
            reflect(unit_vector(r_in.direction()), rec.normal);

        // 直接按理想镜面反射方向生成新光线
        scattered = ray(rec.p, reflected);

        // 金属表面的颜色衰减同样由 albedo 决定
        attenuation = albedo;

        // 只有当反射方向仍然朝表面外侧时，这次散射才算有效
        // 如果点乘 <= 0，说明光线朝物体内部去了，这种情况直接丢弃
        return (dot(scattered.direction(), rec.normal) > 0);
    }
};