#pragma once
#include "ray.h"

class material
{
public:
    virtual bool scatter(
        const ray& r_in,    // 入射光线
        const hit_record& rec,  // 交点信息
        vec3& attenuation,  // 散射后的颜色衰减
        ray& scattered  // 散射后的新光线
    ) const = 0;
};