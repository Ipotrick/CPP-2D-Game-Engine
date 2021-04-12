#pragma once

#include "../types/ShortNames.hpp"

#include <iostream>

#include "basic_math.hpp"
#include "Vec2.hpp"

class Vec3 {
public:
    constexpr Vec3() : 
        x{ 0 }, y{ 0 }, z { 0 } 
    {}

    constexpr Vec3(f32 scalar) :
        x{ scalar }, y{ scalar }, z{ scalar } 
    {}

    constexpr Vec3(f32 x, f32 y, f32 z) :
        x(x), y(y), z(z) 
    {}

    constexpr Vec3(Vec2 xy, f32 z = 0.0f) :
        x{xy.x}, y{xy.y}, z{z} 
    {}

    // openGL access func
    constexpr f32 const* data() const { return &x; }

    constexpr f32& operator[](u32 index) {
        assert(i < 3);
        return (&x)[index];
    }

    constexpr const f32& operator[](u32 index) const {
        assert(i < 3);
        return (&x)[index];
    }

    constexpr Vec3& operator=(Vec2 vec)
    {
        x = vec.x;
        y = vec.y;
        return *this;
    }

public:
    union
    {
        struct
        {
            f32 x; // x coordinate
            f32 y; // y coordinate
            f32 z; // z coordinate
        };

        struct
        {
            f32 r; // red component
            f32 g; // green component
            f32 b; // blue component
        };
    };
};

inline constexpr Vec3& operator*=(Vec3& vec, f32 scalar)
{
    vec.x *= scalar;
    vec.y *= scalar;
    vec.z *= scalar;
    return vec;
}

inline constexpr Vec3& operator*=(Vec3& vecA, Vec2 vecB)
{
    vecA.x *= vecB.x;
    vecA.y *= vecB.y;
    return vecA;
}

inline constexpr Vec3& operator*=(Vec3& vecA, Vec3 vecB)
{
    vecA.x *= vecB.x;
    vecA.y *= vecB.y;
    vecA.z *= vecB.z;
    return vecA;
}

inline constexpr Vec3& operator+=(Vec3& vecA, Vec3 vecB)
{
    vecA.x += vecB.x;
    vecA.y += vecB.y;
    vecA.z += vecB.z;
    return vecA;
}

inline constexpr Vec3& operator+=(Vec3& vecA, Vec2 vecB)
{
    vecA.x += vecB.x;
    vecA.y += vecB.y;
    return vecA;
}

inline constexpr Vec3& operator/=(Vec3 vec, f32 scalar)
{
    vec.x /= scalar;
    vec.y /= scalar;
    vec.z /= scalar;
    return vec;
}

inline constexpr Vec3& operator-=(Vec3& vecA, Vec2 vecB)
{
    vecA.x -= vecB.x;
    vecA.y -= vecB.y;
    return vecA;
}

inline constexpr Vec3& operator-=(Vec3& vecA, Vec3 vecB)
{
    vecA.x -= vecB.x;
    vecA.y -= vecB.y;
    vecA.z -= vecB.z;
    return vecA;
}

inline constexpr Vec3 operator*(Vec3 vecA, Vec2 vecB)
{
    return { vecA.x * vecB.x, vecA.y * vecB.y, vecA.z };
}

inline constexpr Vec3 operator+(Vec3 vecA, Vec2 vecB)
{
    return { vecA.x + vecB.x,
                vecA.y + vecB.y,
                vecA.z };
}

inline constexpr Vec3 operator-(Vec3 vecA, Vec2 vecB)
{
    return { vecA.x - vecB.x,
                vecA.y - vecB.y,
                vecA.z };
}

inline constexpr Vec3 operator-(Vec3 vec) {
    return Vec3(-vec.x, -vec.y, -vec.z);
}

inline constexpr Vec3 operator*(f32 scalar, Vec3 vec) {
    return { scalar * vec.x, scalar * vec.y, scalar * vec.z };
}

inline constexpr Vec3 operator*(Vec3 vec, f32 scalar) {
    return { scalar * vec.x, scalar * vec.y, scalar * vec.z };
}

inline constexpr Vec3 operator*(Vec3 vecA, Vec3 vecB) {
    return { vecA.x * vecB.x, vecA.y * vecB.y, vecA.z * vecB.z };
}

inline constexpr Vec3 operator/(Vec3 vec, f32 scalar) {
    return { vec.x / scalar,
                vec.y / scalar,
                vec.z / scalar };
}

inline constexpr Vec3 operator+(Vec3 vecA, Vec3 vecB) {
    return { vecA.x + vecB.x,
                vecA.y + vecB.y,
                vecA.z + vecB.z };
}

inline constexpr Vec3 operator-(Vec3 vecA, Vec3 vecB) {
    return { vecA.x - vecB.x,
                vecA.y - vecB.y,
                vecA.z - vecB.z };
}

inline constexpr Vec3 min(Vec3 vecA, Vec3 vecB) {
    return { std::min(vecA.x, vecB.x),
                std::min(vecA.y, vecB.y),
                std::min(vecA.z, vecB.z) };
}

inline constexpr Vec3 max(Vec3 vecA, Vec3 vecB) {
    return { std::max(vecA.x, vecB.x),
                std::max(vecA.y, vecB.y),
                std::max(vecA.z, vecB.z) };
}

inline f32 norm(Vec3 vec)
{
    return sqrt(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z);
}

inline f32 length(Vec3 vec)
{
    return sqrt(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z);
}

inline Vec3 normalize(Vec3 vec)
{
    const f32 n = norm(vec);
    if (n != 0.0)
    {
        return Vec3(vec.x / n,
            vec.y / n,
            vec.z / n);
    }
    return vec;
}

// uses trigonometric functions and or sqrt
inline f32 distance(Vec3 vecA, Vec3 vecB) {
    return norm(vecA - vecB);
}

inline constexpr f32 dot(Vec3 vecA, Vec3 vecB) {
    return (vecA.x * vecB.x + vecA.y * vecB.y + vecA.z * vecB.z);
}

inline constexpr Vec3 cross(Vec3 vecA, Vec3 vecB) {
    return { vecA.y * vecB.z - vecA.z * vecB.y,
                vecA.z * vecB.x - vecA.x * vecB.z,
                vecA.x * vecB.y - vecA.y * vecB.x };
}

// reflect vector vec at normal n
inline constexpr Vec3 reflect(Vec3 vec, Vec3 n) {
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