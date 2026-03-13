#pragma once
#include <cmath>
#include <cstdlib>

class vec3 {
public:

    double e[3];

    vec3() : e{ 0,0,0 } {}
    vec3(double e0, double e1, double e2) : e{ e0,e1,e2 } {}

    double x() const { return e[0]; }
    double y() const { return e[1]; }
    double z() const { return e[2]; }

    vec3 operator-() const {
        return vec3(-e[0], -e[1], -e[2]);
    }

    double operator[](int i) const { return e[i]; }
    double& operator[](int i) { return e[i]; }

    vec3& operator+=(const vec3& v) {
        e[0] += v.e[0];
        e[1] += v.e[1];
        e[2] += v.e[2];
        return *this;
    }

    vec3& operator*=(double t) {
        e[0] *= t;
        e[1] *= t;
        e[2] *= t;
        return *this;
    }

    vec3& operator/=(double t) {
        return *this *= 1.0 / t;
    }

    double length() const {
        return std::sqrt(length_squared());
    }

    double length_squared() const {
        return e[0] * e[0] + e[1] * e[1] + e[2] * e[2];
    }

    bool near_zero() const {
        const double s = 1e-8;
        return (fabs(e[0]) < s) && (fabs(e[1]) < s) && (fabs(e[2]) < s);
    }
};

// 向量运算
inline vec3 operator+(const vec3& u, const vec3& v) {
    return vec3(
        u.e[0] + v.e[0],
        u.e[1] + v.e[1],
        u.e[2] + v.e[2]
    );
}

inline vec3 operator-(const vec3& u, const vec3& v) {
    return vec3(
        u.e[0] - v.e[0],
        u.e[1] - v.e[1],
        u.e[2] - v.e[2]
    );
}

inline vec3 operator*(const vec3& u, const vec3& v) {
    return vec3(
        u.e[0] * v.e[0],
        u.e[1] * v.e[1],
        u.e[2] * v.e[2]
    );
}

inline vec3 operator*(double t, const vec3& v) {
    return vec3(
        t * v.e[0],
        t * v.e[1],
        t * v.e[2]
    );
}

inline vec3 operator*(const vec3& v, double t) {
    return t * v;
}

inline vec3 operator/(vec3 v, double t) {
    return (1 / t) * v;
}

// 向量数学
inline double dot(const vec3& u, const vec3& v) {
    return u.e[0] * v.e[0]
        + u.e[1] * v.e[1]
        + u.e[2] * v.e[2];
}

inline vec3 cross(const vec3& u, const vec3& v) {
    return vec3(
        u.e[1] * v.e[2] - u.e[2] * v.e[1],
        u.e[2] * v.e[0] - u.e[0] * v.e[2],
        u.e[0] * v.e[1] - u.e[1] * v.e[0]
    );
}

inline vec3 unit_vector(vec3 v) {
    return v / v.length();
}

// 随机数工具
inline double random_double() {
    return rand() / (RAND_MAX + 1.0);
}

inline double random_double(double min, double max) {
    return min + (max - min) * random_double();
}

// 随机向量
inline vec3 random_vec3() {
    return vec3(random_double(), random_double(), random_double());
}

inline vec3 random_vec3(double min, double max) {
    return vec3(
        random_double(min, max),
        random_double(min, max),
        random_double(min, max)
    );
}

inline vec3 random_in_unit_sphere() {
    while (true) {
        vec3 p = random_vec3(-1, 1);
        if (p.length_squared() >= 1) continue;
        return p;
    }
}

inline vec3 random_unit_vector() {
    return unit_vector(random_in_unit_sphere());
}

inline vec3 reflect(const vec3& v, const vec3& n)
{
    return v - 2 * dot(v, n) * n;
}

// 折射函数：uv：入射方向；etai_over_etat：折射率比
inline vec3 refract(const vec3& uv, const vec3& n, double etai_over_etat)
{
    double cos_theta = fmin(dot(-uv, n), 1.0);
    // 垂直分量
    vec3 r_out_perp = etai_over_etat * (uv + cos_theta * n);
    // 平行分量
    vec3 r_out_parallel =
        -sqrt(fabs(1.0 - r_out_perp.length_squared())) * n;

    return r_out_perp + r_out_parallel;
}