#pragma once
#include "vec3.h"

class ray {
public:
    vec3 orig;  // 光线起点
    vec3 dir;   // 光线方向

    ray() {}

    ray(const vec3& origin, const vec3& direction)
        : orig(origin), dir(direction) {
    }

    vec3 origin() const { return orig; }
    vec3 direction() const { return dir; }

    vec3 at(double t) const {
        // 返回光线在参数 t 处对应的空间位置：P(t) = O + tD
        return orig + dir * t;
    }
};