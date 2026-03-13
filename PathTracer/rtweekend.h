#pragma once
#include <cmath>
#include <limits>
#include <memory>
#include <cstdlib>

const double infinity = std::numeric_limits<double>::infinity();
const double pi = 3.1415926535897932385;


// 角度转弧度
inline double degrees_to_radians(double degrees)
{
    return degrees * pi / 180.0;
}


// 随机数 [0,1)
inline double random_double()
{
    return rand() / (RAND_MAX + 1.0);
}


// 随机数 [min,max)
inline double random_double(double min, double max)
{
    return min + (max - min) * random_double();
}


// 限制范围
inline double clamp(double x, double min, double max)
{
    if (x < min) return min;
    if (x > max) return max;
    return x;
}