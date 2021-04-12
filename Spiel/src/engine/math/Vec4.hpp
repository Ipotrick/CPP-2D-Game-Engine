#pragma once

#include "../types/ShortNames.hpp"

#include <iostream>

#include "basic_math.hpp"

struct Vec4
{
    constexpr Vec4() : x(0), y(0), z(0), w(0) {}

    constexpr Vec4(f32 x, f32 y, f32 z, f32 w) : x(x), y(y), z(z), w(w) {}

    constexpr static Vec4 From255(u8 r, u8 g, u8 b, u8 a = 255)
    {
        return Vec4{ static_cast<f32>(r) / 255.0f, static_cast<f32>(g) / 255.0f, static_cast<f32>(b) / 255.0f, static_cast<f32>(a) / 255.0f };
    }

    // openGL access func
    constexpr f32 const* data() const { return &x; }

    constexpr f32& operator[](u32 _i)
    {
        assert(_i < 4);
        return (&x)[_i];
    }

    constexpr f32 const operator[](u32 _i) const
    {
        assert(_i < 4);
        return (&x)[_i];
    }

    Vec4& operator*=(f32 const scalar)
    {
        x *= scalar;
        y *= scalar;
        z *= scalar;
        w *= scalar;
        return *this;
    }

    Vec4& operator/=(f32 const scalar)
    {
        x /= scalar;
        y /= scalar;
        z /= scalar;
        w /= scalar;
        return *this;
    }

    Vec4& operator*=(Vec4 const& vec)
    {
        x *= vec.x;
        y *= vec.y;
        z *= vec.z;
        w *= vec.w;
        return *this;
    }

    Vec4& operator-=(Vec4 const& vec)
    {
        x -= vec.x;
        y -= vec.y;
        z -= vec.z;
        w -= vec.w;
        return *this;
    }

    Vec4& operator+=(Vec4 const& vec)
    {
        x += vec.x;
        y += vec.y;
        z += vec.z;
        w += vec.w;
        return *this;
    }

    bool operator==(const Vec4& rhs) const
    {
        return this->x == rhs.x && 
            this->y == rhs.y && 
            this->z == rhs.z && 
            this->w == rhs.w;
    }

    void colorInvert()
    {
        x = 1.0f - x;
        y = 1.0f - y;
        z = 1.0f - z;
        w = 1.0f - w;
    }

    union {
        struct {
            f32 x; // x coordinate
            f32 y; // y coordinate
            f32 z; // z coordinate
            f32 w; // w coordinate
        };

        struct {
            f32 r; // red component
            f32 g; // green component
            f32 b; // blue component
            f32 a; // alpha component
        };
    };
};

inline bool hasNANS(const Vec4& vec)
{
    return std::isnan(vec.x) || std::isnan(vec.y) || std::isnan(vec.z) || std::isnan(vec.w);
}

inline constexpr  Vec4 operator-(Vec4 const& vec)
{
    return Vec4(-vec.x, -vec.y, -vec.z, -vec.w);
}

inline constexpr  Vec4 operator*(f32 const scalar, Vec4 const& vec)
{
    return Vec4(scalar * vec.x,
        scalar * vec.y,
        scalar * vec.z,
        scalar * vec.w);
}

inline constexpr  Vec4 operator*(Vec4 const& vec, f32 const scalar)
{
    return Vec4(scalar * vec.x,
        scalar * vec.y,
        scalar * vec.z,
        scalar * vec.w);
}

inline constexpr  Vec4 operator*(Vec4 const& vecA, Vec4 const& vecB)
{
    return Vec4(vecA.x * vecB.x,
        vecA.y * vecB.y,
        vecA.z * vecB.z,
        vecA.w * vecB.w);
}

inline constexpr  Vec4 operator/(Vec4 const& vec, f32 const scalar)
{
    return Vec4(vec.x / scalar,
        vec.y / scalar,
        vec.z / scalar,
        vec.w / scalar);
}

inline constexpr  Vec4 operator+(Vec4 const& vecA, Vec4 const& vecB)
{
    return Vec4(vecA.x + vecB.x,
        vecA.y + vecB.y,
        vecA.z + vecB.z,
        vecA.w + vecB.w);
}

inline constexpr  Vec4 operator-(Vec4 const& vecA, Vec4 const& vecB)
{
    return Vec4(vecA.x - vecB.x,
        vecA.y - vecB.y,
        vecA.z - vecB.z,
        vecA.w - vecB.w);
}

inline f32 norm(Vec4 const& vec)
{
    return sqrt(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z + vec.w * vec.w);
}

inline f32 length(Vec4 const& vec)
{
    return sqrt(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z + vec.w * vec.w);
}

inline f32 dot(Vec4 const& vecA, Vec4 const& vecB)
{
    return (vecA.x * vecB.x + vecA.y * vecB.y + vecA.z * vecB.z + vecA.w * vecB.w);
}

inline std::istream& operator>>(std::istream& is, Vec4& vec)
{
    is >> vec.x >> vec.y >> vec.z >> vec.w;
    return is;
}

inline std::ostream& operator<<(std::ostream& os, Vec4 const& vec)
{
    os << '(' << vec.x << ", " << vec.y << ", " << vec.z << ", " << vec.w << ')';
    return os;
}