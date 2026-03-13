#pragma once
#include "material.h"

class lambertian : public material
{
public:

    vec3 albedo;

    lambertian(const vec3& a) :albedo(a) {}

    virtual bool scatter(
        const ray& r_in,
        const hit_record& rec,
        vec3& attenuation,
        ray& scattered
    ) const override
    {
        vec3 scatter_direction = rec.normal + random_unit_vector();

        if (scatter_direction.near_zero())
            scatter_direction = rec.normal;

        scattered = ray(rec.p, scatter_direction);
        attenuation = albedo;

        return true;
    }
};