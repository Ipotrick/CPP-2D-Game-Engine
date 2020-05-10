#pragma once

#include <iostream>

#include "basic_math.h"

class Vec4
{
public:

    /// this union allows us to access vector elements by either x,y,z or r,g,b,
    /// but we have to store only three floats.
    union
    {
        /// XYZW coordinates
        struct
        {
            float x; ///< x coordinate
            float y; ///< y coordinate
            float z; ///< z coordinate
            float w; ///< w coordinate
        };

        /// RGBA color components
        struct
        {
            float r; ///< red component
            float g; ///< green component
            float b; ///< blue component
            float a; ///< alpha component
        };
    };


public:

    Vec4() : x(0), y(0), z(0), w(0) {}

    Vec4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}

    // openGL access func
    float const* data() const { return &x; }

    float& operator[](unsigned int _i)
    {
        assert(_i < 4);
        return (&x)[_i];
    }

    float const operator[](unsigned int _i) const
    {
        assert(_i < 4);
        return (&x)[_i];
    }

    Vec4& operator*=(float const scalar)
    {
        x *= scalar;
        y *= scalar;
        z *= scalar;
        w *= scalar;
        return *this;
    }

    Vec4& operator/=(float const scalar)
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
};

inline const Vec4 operator-(Vec4 const& vec)
{
    return Vec4(-vec.x, -vec.y, -vec.z, -vec.w);
}

inline const Vec4 operator*(float const scalar, Vec4 const& vec)
{
    return Vec4(scalar * vec.x,
        scalar * vec.y,
        scalar * vec.z,
        scalar * vec.w);
}

inline const Vec4 operator*(Vec4 const& vec, float const scalar)
{
    return Vec4(scalar * vec.x,
        scalar * vec.y,
        scalar * vec.z,
        scalar * vec.w);
}

inline const Vec4 operator*(Vec4 const& vecA, Vec4 const& vecB)
{
    return Vec4(vecA.x * vecB.x,
        vecA.y * vecB.y,
        vecA.z * vecB.z,
        vecA.w * vecB.w);
}

inline const Vec4 operator/(Vec4 const& vec, float const scalar)
{
    return Vec4(vec.x / scalar,
        vec.y / scalar,
        vec.z / scalar,
        vec.w / scalar);
}

inline const Vec4 operator+(Vec4 const& vecA, Vec4 const& vecB)
{
    return Vec4(vecA.x + vecB.x,
        vecA.y + vecB.y,
        vecA.z + vecB.z,
        vecA.w + vecB.w);
}

inline const Vec4 operator-(Vec4 const& vecA, Vec4 const& vecB)
{
    return Vec4(vecA.x - vecB.x,
        vecA.y - vecB.y,
        vecA.z - vecB.z,
        vecA.w - vecB.w);
}

inline float const norm(Vec4 const& vec)
{
    return sqrt(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z + vec.w * vec.w);
}

inline float const length(Vec4 const& vec)
{
    return sqrt(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z + vec.w * vec.w);
}

inline float const dot(Vec4 const& vecA, Vec4 const& vecB)
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