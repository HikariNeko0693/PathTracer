#pragma once

#include "hittable.h"
#include "hittable_list.h"
#include "aabb.h"

#include <algorithm>

class bvh_node : public hittable
{
public:

    std::shared_ptr<hittable> left;
    std::shared_ptr<hittable> right;
    aabb box;

    bvh_node() {}

    bvh_node(
        std::vector<std::shared_ptr<hittable>>& objects,
        size_t start,
        size_t end
    )
    {
        int axis = int(3 * random_double());

        auto comparator =
            (axis == 0) ? box_x_compare
            : (axis == 1) ? box_y_compare
            : box_z_compare;

        size_t object_span = end - start;

        if (object_span == 1)
        {
            left = right = objects[start];
        }
        else if (object_span == 2)
        {
            if (comparator(objects[start], objects[start + 1]))
            {
                left = objects[start];
                right = objects[start + 1];
            }
            else
            {
                left = objects[start + 1];
                right = objects[start];
            }
        }
        else
        {
            std::sort(
                objects.begin() + start,
                objects.begin() + end,
                comparator
            );

            auto mid = start + object_span / 2;

            left = std::make_shared<bvh_node>(objects, start, mid);
            right = std::make_shared<bvh_node>(objects, mid, end);
        }

        aabb box_left, box_right;

        left->bounding_box(box_left);
        right->bounding_box(box_right);

        box = surrounding_box(box_left, box_right);
    }

    virtual bool hit(
        const ray& r,
        double t_min,
        double t_max,
        hit_record& rec
    ) const override
    {
        if (!box.hit(r, t_min, t_max))
            return false;

        bool hit_left = left->hit(r, t_min, t_max, rec);

        bool hit_right = right->hit(
            r,
            t_min,
            hit_left ? rec.t : t_max,
            rec
        );

        return hit_left || hit_right;
    }

    virtual bool bounding_box(aabb& output_box) const override
    {
        output_box = box;
        return true;
    }

private:

    static bool box_compare(
        const std::shared_ptr<hittable> a,
        const std::shared_ptr<hittable> b,
        int axis
    )
    {
        aabb box_a;
        aabb box_b;

        a->bounding_box(box_a);
        b->bounding_box(box_b);

        return box_a.min()[axis] < box_b.min()[axis];
    }

    static bool box_x_compare(
        const std::shared_ptr<hittable> a,
        const std::shared_ptr<hittable> b
    )
    {
        return box_compare(a, b, 0);
    }

    static bool box_y_compare(
        const std::shared_ptr<hittable> a,
        const std::shared_ptr<hittable> b
    )
    {
        return box_compare(a, b, 1);
    }

    static bool box_z_compare(
        const std::shared_ptr<hittable> a,
        const std::shared_ptr<hittable> b
    )
    {
        return box_compare(a, b, 2);
    }
};