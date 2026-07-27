#pragma once
#include <cmath>
#include <random>

class vec3 {
public:
    double e[3];

    vec3() : e{ 0,0,0 } {}
    vec3(double e0, double e1, double e2) : e{ e0,e1,e2 } {}

    double x() const { return e[0]; }
    double y() const { return e[1]; }
    double z() const { return e[2]; }

    // 取反
    vec3 operator-() const {
        return vec3(-e[0], -e[1], -e[2]);
    }

    // 下标访问
    double operator[](int i) const { return e[i]; }
    double& operator[](int i) { return e[i]; }

    // 向量加法赋值
    vec3& operator+=(const vec3& v) {
        e[0] += v.e[0];
        e[1] += v.e[1];
        e[2] += v.e[2];
        return *this;
    }

    // 标量乘法赋值
    vec3& operator*=(double t) {
        e[0] *= t;
        e[1] *= t;
        e[2] *= t;
        return *this;
    }

    // 标量除法赋值
    vec3& operator/=(double t) {
        return *this *= 1.0 / t;
    }

    // 向量长度
    double length() const {
        return std::sqrt(length_squared());
    }

    // 向量长度平方
    double length_squared() const {
        return e[0] * e[0] + e[1] * e[1] + e[2] * e[2];
    }

    // 判断是否接近零向量
    bool near_zero() const {
        const double s = 1e-8;
        return (fabs(e[0]) < s) && (fabs(e[1]) < s) && (fabs(e[2]) < s);
    }
};


// 向量加法
inline vec3 operator+(const vec3& u, const vec3& v) {
    return vec3(
        u.e[0] + v.e[0],
        u.e[1] + v.e[1],
        u.e[2] + v.e[2]
    );
}

// 向量减法
inline vec3 operator-(const vec3& u, const vec3& v) {
    return vec3(
        u.e[0] - v.e[0],
        u.e[1] - v.e[1],
        u.e[2] - v.e[2]
    );
}

// 分量乘法，颜色衰减时用
inline vec3 operator*(const vec3& u, const vec3& v) {
    return vec3(
        u.e[0] * v.e[0],
        u.e[1] * v.e[1],
        u.e[2] * v.e[2]
    );
}

// 标量 * 向量
inline vec3 operator*(double t, const vec3& v) {
    return vec3(
        t * v.e[0],
        t * v.e[1],
        t * v.e[2]
    );
}

// 向量 * 标量
inline vec3 operator*(const vec3& v, double t) {
    return t * v;
}

// 向量 / 标量
inline vec3 operator/(vec3 v, double t) {
    return (1 / t) * v;
}


// 点积
inline double dot(const vec3& u, const vec3& v) {
    return u.e[0] * v.e[0]
        + u.e[1] * v.e[1]
        + u.e[2] * v.e[2];
}

// 叉积（构建正交坐标系）
inline vec3 cross(const vec3& u, const vec3& v) {
    return vec3(
        u.e[1] * v.e[2] - u.e[2] * v.e[1],
        u.e[2] * v.e[0] - u.e[0] * v.e[2],
        u.e[0] * v.e[1] - u.e[1] * v.e[0]
    );
}

// 归一化
inline vec3 unit_vector(vec3 v) {
    return v / v.length();
}


inline std::mt19937& random_generator()
{
    static thread_local std::mt19937 generator(1234);
    return generator;
}

inline void set_random_seed(unsigned int seed)
{
    random_generator().seed(seed);
}

// 生成 [0,1) 随机数
inline double random_double() {
    static thread_local std::uniform_real_distribution<double> distribution(0.0, 1.0);
    return distribution(random_generator());
}

// 生成 [min,max) 随机数
inline double random_double(double min, double max) {
    return min + (max - min) * random_double();
}


// 每个分量都在 [0,1) 范围内
inline vec3 random_vec3() {
    return vec3(random_double(), random_double(), random_double());
}

// 每个分量都在 [min,max) 范围内
inline vec3 random_vec3(double min, double max) {
    return vec3(
        random_double(min, max),
        random_double(min, max),
        random_double(min, max)
    );
}

// 在单位球内随机采样一个点（漫反射采样）
inline vec3 random_in_unit_sphere() {
    while (true) {
        vec3 p = random_vec3(-1, 1);

        // 如果落在单位球外，就重采样
        if (p.length_squared() >= 1) continue;

        return p;
    }
}

// 生成单位长度的随机方向向量
inline vec3 random_unit_vector() {
    return unit_vector(random_in_unit_sphere());
}

// 镜面反射方向
inline vec3 reflect(const vec3& v, const vec3& n)
{
    return v - 2 * dot(v, n) * n;
}

// 折射方向
// uv：单位化后的入射方向
// etai_over_etat：折射率比
inline vec3 refract(const vec3& uv, const vec3& n, double etai_over_etat)
{
    double cos_theta = fmin(dot(-uv, n), 1.0);

    // 折射方向中垂直于法线的分量
    vec3 r_out_perp = etai_over_etat * (uv + cos_theta * n);

    // 平行于法线的分量
    vec3 r_out_parallel =
        -sqrt(fabs(1.0 - r_out_perp.length_squared())) * n;

    return r_out_perp + r_out_parallel;
}

// 在单位圆盘中随机采样，用于相机镜头采样，实现景深
inline vec3 random_in_unit_disk()
{
    while (true)
    {
        vec3 p(
            random_double() * 2 - 1,
            random_double() * 2 - 1,
            0
        );

        if (p.length_squared() >= 1) continue;

        return p;
    }
}