#pragma once

#include <iostream>

#include "basic_math.h"

class Vec3 {
public:
    Vec3() : x{ 0 }, y{ 0 }, z { 0 } {}
    Vec3(float scalar) : x{ scalar }, y{ scalar }, z{ scalar } {}
    Vec3(float x, float y, float z) : x(x), y(y), z(z) {}

    // openGL access func
    float const* data() const { return &x; }

    float& operator[](unsigned i) {
        assert(i < 3);
        return (&x)[i];
    }

    float const operator[](unsigned i) const
    {
        assert(i < 3);
        return (&x)[i];
    }

    Vec3& operator*=(float const scalar) {
        x *= scalar;
        y *= scalar;
        z *= scalar;
        return *this;
    }

    Vec3& operator/=(float const scalar) {
        x /= scalar;
        y /= scalar;
        z /= scalar;
        return *this;
    }

    Vec3& operator*=(Vec3 const& vec) {
        x *= vec.x;
        y *= vec.y;
        z *= vec.z;
        return *this;
    }

    Vec3& operator-=(Vec3 const& vec) {
        x -= vec.x;
        y -= vec.y;
        z -= vec.z;
        return *this;
    }

    Vec3& operator+=(Vec3 const& vec) {
        x += vec.x;
        y += vec.y;
        z += vec.z;
        return *this;
    }
public:
    union
    {
        struct
        {
            float x; // x coordinate
            float y; // y coordinate
            float z; // z coordinate
        };

        struct
        {
            float r; // red component
            float g; // green component
            float b; // blue component
        };
    };
};

inline Vec3 const operator-(Vec3 const& vec) {
    return Vec3(-vec.x, -vec.y, -vec.z);
}

inline Vec3 const operator*(float const scalar, Vec3 const& vec) {
    return { scalar * vec.x, scalar * vec.y, scalar * vec.z };
}

inline Vec3 const operator*(Vec3 const& vec, float const scalar) {
    return { scalar * vec.x, scalar * vec.y, scalar * vec.z };
}

inline Vec3 const operator*(Vec3 const& vecA, Vec3 const& vecB) {
    return { vecA.x * vecB.x, vecA.y * vecB.y, vecA.z * vecB.z };
}

inline Vec3 const operator/(Vec3 const& vec, float const scalar) {
    return { vec.x / scalar,
                vec.y / scalar,
                vec.z / scalar };
}

inline Vec3 const operator+(Vec3 const& vecA, Vec3 const& vecB) {
    return { vecA.x + vecB.x,
                vecA.y + vecB.y,
                vecA.z + vecB.z };
}

inline Vec3 const operator-(Vec3 const& vecA, Vec3 const& vecB) {
    return { vecA.x - vecB.x,
                vecA.y - vecB.y,
                vecA.z - vecB.z };
}

inline Vec3 const min(Vec3 const& vecA, Vec3 const& vecB) {
    return { std::min(vecA.x, vecB.x),
                std::min(vecA.y, vecB.y),
                std::min(vecA.z, vecB.z) };
}

inline Vec3 const max(Vec3 const& vecA, Vec3 const& vecB) {
    return { std::max(vecA.x, vecB.x),
                std::max(vecA.y, vecB.y),
                std::max(vecA.z, vecB.z) };
}

// uses trigonometric functions and or sqrt
inline const float norm(Vec3 const& vec)
{
    return sqrt(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z);
}

// uses trigonometric functions and or sqrt
inline const float length(Vec3 const& vec)
{
    return sqrt(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z);
}

// uses trigonometric functions and or sqrt
inline const Vec3 normalize(Vec3 const& vec)
{
    const float n = norm(vec);
    if (n != 0.0)
    {
        return Vec3(vec.x / n,
            vec.y / n,
            vec.z / n);
    }
    return vec;
}

// uses trigonometric functions and or sqrt
inline float const distance(Vec3 const& vecA, Vec3 const& vecB) {
    return norm(vecA - vecB);
}

inline float const dot(Vec3 const& vecA, Vec3 const& vecB) {
    return (vecA.x * vecB.x + vecA.y * vecB.y + vecA.z * vecB.z);
}

inline Vec3 const cross(Vec3 const& vecA, Vec3 const& vecB) {
    return { vecA.y * vecB.z - vecA.z * vecB.y,
                vecA.z * vecB.x - vecA.x * vecB.z,
                vecA.x * vecB.y - vecA.y * vecB.x };
}

// reflect vector vec at normal n
inline Vec3 const reflect(Vec3 const& vec, Vec3 const& n) {
    return vec - (2.0f * dot(n, vec)) * n;
}

inline std::istream& operator>>(std::istream& is, Vec3& vec) {
    is >> vec.x >> vec.y >> vec.z;
    return is;
}

inline std::ostream& operator<<(std::ostream& os, Vec3 const& vec) {
    os << '(' << vec.x << ", " << vec.y << ", " << vec.z << ')';
    return os;
}