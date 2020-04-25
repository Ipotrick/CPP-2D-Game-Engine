#pragma once

#include "Vec.h"

class Mat4 {
public:
    Mat4() : data_{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0} {}

    float& operator()(const int i, const int j) { return data_[i + (j << 2)]; }

    float operator()(const int i, const int j) const { return data_[i + (j << 2)]; }

    // openGL access func
    const float* data() const { return data_; }

    // return identity matrix
    static Mat4 identity();
    // return frustum matrix
    static Mat4 frustum(float left, float right, float bottom, float top, float near, float far);
    // return matrix for perspective projection (special case of frustum matrix)
    static Mat4 perspective(float fovy, float aspect, float near, float far);
    // return look-at camera matrix
    static Mat4 look_at(Vec3 const& eye, Vec3 const& center, Vec3 const& up);
    // return translation matrix
    static Mat4 translate(Vec3 const& t);
    // return scaling matrix
    static Mat4 scale(float factor);
    // return scaling matrix
    static Mat4 scale(Vec3 scale);
    // return matrix for rotation around x-axis
    static Mat4 rotate_x(float angle);
    // return matrix for rotation around y-axis
    static Mat4 rotate_y(float angle);
    // return matrix for rotation around z-axis
    static Mat4 rotate_z(float angle);
private:
    float data_[16];
};


//-----------------------------------------------------------------------------


/// return matrix product matA*matB
Mat4 operator*(Mat4 const& matA, Mat4 const& matB);

/// return matrix-vector product mat*v
Vec4 operator*(Mat4 const& mat, const Vec4& v0);

/// print matrix to output stream
std::ostream& operator<<(std::ostream& os, Mat4 const& mat);