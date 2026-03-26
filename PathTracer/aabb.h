#pragma once

#include "ray.h"
#include "vec3.h"
#include <algorithm>

class aabb
{
public:
    vec3 minimum;  // 盒子最小顶点
    vec3 maximum;  // 盒子最大顶点

    aabb() {}

    aabb(const vec3& a, const vec3& b)
        : minimum(a), maximum(b) {
    }

    vec3 min() const { return minimum; }
    vec3 max() const { return maximum; }

    // 判断光线是否与当前 AABB 相交
    bool hit(const ray& r, double t_min, double t_max) const
    {
        // 分别在 x / y / z 三个轴上做“区间裁剪”
        for (int a = 0; a < 3; a++)
        {
            // invD = 1 / direction[a]
            // 这样后面可以把除法转成乘法
            auto invD = 1.0f / r.direction()[a];

            // 计算光线与当前轴两个平面的参数值 t0 / t1
            auto t0 = (min()[a] - r.origin()[a]) * invD;
            auto t1 = (max()[a] - r.origin()[a]) * invD;

            // 如果方向为负，说明先撞到的是 max 平面，后撞到的是 min 平面
            // 所以要交换，保证 t0 始终表示“进入”，t1 表示“离开”
            if (invD < 0.0)
                std::swap(t0, t1);

            // 用当前轴的交区间，收缩全局可行区间
            t_min = t0 > t_min ? t0 : t_min;
            t_max = t1 < t_max ? t1 : t_max;

            // 只要某一轴上区间已经空了，就说明整条光线不可能穿过这个盒子
            if (t_max <= t_min)
                return false;
        }

        return true;
    }
};

// 求两个包围盒的并集包围盒，用于自底向上构建 BVH 节点的总包围盒
inline aabb surrounding_box(aabb box0, aabb box1)
{
    vec3 small(
        fmin(box0.min().x(), box1.min().x()),
        fmin(box0.min().y(), box1.min().y()),
        fmin(box0.min().z(), box1.min().z())
    );

    vec3 big(
        fmax(box0.max().x(), box1.max().x()),
        fmax(box0.max().y(), box1.max().y()),
        fmax(box0.max().z(), box1.max().z())
    );

    return aabb(small, big);
}