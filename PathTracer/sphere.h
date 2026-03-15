#pragma once
#include "hittable.h"
#include "aabb.h"
#include <memory>

class sphere : public hittable
{
public:

    vec3 center;
    double radius = 0;
    std::shared_ptr<material> mat_ptr;

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
        vec3 oc = r.origin() - center;

        auto a = r.direction().length_squared();
        auto half_b = dot(oc, r.direction());
        auto c = oc.length_squared() - radius * radius;

        auto discriminant = half_b * half_b - a * c;

        if (discriminant < 0) return false;

        auto sqrtd = sqrt(discriminant);

        auto root = (-half_b - sqrtd) / a;

        if (root < t_min || root > t_max)
        {
            root = (-half_b + sqrtd) / a;

            if (root < t_min || root > t_max)
                return false;
        }

        rec.t = root;
        rec.p = r.at(rec.t);

        vec3 outward_normal = (rec.p - center) / radius;

        rec.set_face_normal(r, outward_normal);

        rec.mat_ptr = mat_ptr;

        return true;
    }

    virtual bool bounding_box(aabb& output_box) const override
    {
        output_box = aabb(
            center - vec3(radius, radius, radius),
            center + vec3(radius, radius, radius)
        );

        return true;
    }
};