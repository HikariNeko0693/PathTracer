#pragma once
#include "material.h"

// reflectance：Schlick 近似
// 用于根据入射角和折射率近似计算“当前更偏向反射还是折射”
inline double reflectance(double cosine, double ref_idx)
{
    // 垂直入射时的基础反射率 r0
    double r0 = (1 - ref_idx) / (1 + ref_idx);
    r0 = r0 * r0;

    // Schlick approximation：
    // 入射角越刁钻（越接近掠射），反射概率越高
    return r0 + (1 - r0) * pow((1 - cosine), 5);
}

class dielectric : public material
{
public:
    double ir;  // 折射率

    dielectric(double index) : ir(index) {}

    virtual bool scatter(
        const ray& r_in,
        const hit_record& rec,
        vec3& attenuation,
        ray& scattered
    ) const override
    {
        // 玻璃本身在这个简化模型里不吸收颜色，所以颜色衰减设为白色，相当于只改变传播方向，不主动染色
        attenuation = vec3(1, 1, 1);

        // 根据当前命中的是外表面还是内表面，决定折射率比值
        // front_face = true  : 空气 -> 玻璃，用 1.0 / ir
        // front_face = false : 玻璃 -> 空气，用 ir
        double refraction_ratio =
            rec.front_face ? (1.0 / ir) : ir;

        vec3 unit_direction =
            unit_vector(r_in.direction());

        // cos_theta 表示入射方向与法线夹角的余弦值
        double cos_theta = fmin(dot(-unit_direction, rec.normal), 1.0);  // 取 min(..., 1.0) ：数值稳定
        double sin_theta = sqrt(1.0 - cos_theta * cos_theta);

        // 判断是否发生全反射：refraction_ratio * sin_theta > 1
        bool cannot_refract = refraction_ratio * sin_theta > 1.0;

        vec3 direction;

        // 两种情况会走反射分支：
        // 1. 发生全反射，根本不能折射
        // 2. 虽然可以折射，但根据 Schlick 近似，本次随机决定走反射
        if (cannot_refract ||
            reflectance(cos_theta, refraction_ratio) > random_double())
        {
            direction = reflect(unit_direction, rec.normal);
        }
        else
        {
            // 否则走折射分支
            direction = refract(unit_direction, rec.normal, refraction_ratio);
        }

        // 从交点出发，沿最终选定的方向生成新光线
        scattered = ray(rec.p, direction);

        return true;  // 玻璃材质会继续传播光线
    }
};