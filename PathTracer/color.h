#pragma once
#include "vec3.h"
#include <iostream>
#include <algorithm>

void write_color(std::ostream& out, vec3 pixel_color, int samples_per_pixel)
{
    double scale = 1.0 / samples_per_pixel;

    double r = sqrt(pixel_color.x() * scale);
    double g = sqrt(pixel_color.y() * scale);
    double b = sqrt(pixel_color.z() * scale);

    out << static_cast<int>(256 * std::clamp(r, 0.0, 0.999)) << ' '
        << static_cast<int>(256 * std::clamp(g, 0.0, 0.999)) << ' '
        << static_cast<int>(256 * std::clamp(b, 0.0, 0.999)) << '\n';
}