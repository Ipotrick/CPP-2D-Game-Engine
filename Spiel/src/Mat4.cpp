#include "Mat4.h"

Vec4 operator*(Mat4 const& m, Vec4 const& v0)
{
    Vec4 v;

    for (int i = 0; i < 4; ++i)
    {
        v[i] = 0.0;
        for (int j = 0; j < 4; ++j)
        {
            v[i] += m(i, j) * v0[j];
        }
    }

    return v;
}

Mat4 operator*(Mat4 const& m0, Mat4 const& m1)
{
    Mat4 m;

    for (int i = 0; i < 4; ++i)
    {
        for (int j = 0; j < 4; ++j)
        {
            m(i, j) = 0.0f;
            for (int k = 0; k < 4; ++k)
                m(i, j) += m0(i, k) * m1(k, j);
        }
    }

    return m;
}


//-----------------------------------------------------------------------------



Mat4 Mat4::identity()
{
    Mat4 m;
    m(0, 0) = m(1, 1) = m(2, 2) = m(3, 3) = 1.0f;

    return m;
}

Mat4 Mat4::frustum(float l, float r, float b, float t, float n, float f)
{
    Mat4 m;
    m(0, 0) = 2.0f * n / (r - l);
    m(1, 1) = 2.0f * n / (t - b);
    m(0, 2) = (r + l) / (r - l);
    m(1, 2) = (t + b) / (t - b);
    m(2, 2) = (n - f) / (f - n);
    m(3, 2) = -1.0f;
    m(2, 3) = -2.0f * f * n / (f - n);

    return m;
}

Mat4 Mat4::perspective(float fovy, float aspect, float near, float far)
{
    float t = near * tan(fovy * (float)M_PI / 360.0f);
    float b = -t;
    float l = b * aspect;
    float r = t * aspect;

    return Mat4::frustum(l, r, b, t, near, far);
}

Mat4 Mat4::look_at(Vec3 const& eye, Vec3 const& center, Vec3 const& up)
{
    Vec3 z = normalize(eye - center);
    Vec3 x = normalize(cross(up, z));
    Vec3 y = normalize(cross(z, x));

    Mat4 m;
    m(0, 0) = x[0]; m(0, 1) = x[1]; m(0, 2) = x[2]; m(0, 3) = -dot(x, eye);
    m(1, 0) = y[0]; m(1, 1) = y[1]; m(1, 2) = y[2]; m(1, 3) = -dot(y, eye);
    m(2, 0) = z[0]; m(2, 1) = z[1]; m(2, 2) = z[2]; m(2, 3) = -dot(z, eye);
    m(3, 0) = 0.0;  m(3, 1) = 0.0;  m(3, 2) = 0.0;  m(3, 3) = 1.0;

    return m;
}

Mat4 Mat4::translate(Vec3 const& t)
{
    Mat4 m;
    m(0, 0) = m(1, 1) = m(2, 2) = m(3, 3) = 1.0f;
    m(0, 3) = t[0];
    m(1, 3) = t[1];
    m(2, 3) = t[2];

    return m;
}

Mat4 Mat4::rotate_x(float angle)
{
    float ca = cosf(angle * ((float)M_PI / 180.0f));
    float sa = sinf(angle * ((float)M_PI / 180.0f));

    Mat4 m;
    m(0, 0) = 1.0f;
    m(1, 1) = ca;
    m(1, 2) = -sa;
    m(2, 2) = ca;
    m(2, 1) = sa;
    m(3, 3) = 1.0f;

    return m;
}

Mat4 Mat4::rotate_y(float angle)
{
    float ca = cosf(angle * ((float)M_PI / 180.0f));
    float sa = sinf(angle * ((float)M_PI / 180.0f));

    Mat4 m;
    m(0, 0) = ca;
    m(0, 2) = sa;
    m(1, 1) = 1.0f;
    m(2, 0) = -sa;
    m(2, 2) = ca;
    m(3, 3) = 1.0f;

    return m;
}

Mat4 Mat4::rotate_z(float angle)
{
    float ca = cosf(angle * ((float)M_PI / 180.0f));
    float sa = sinf(angle * ((float)M_PI / 180.0f));

    Mat4 m;
    m(0, 0) = ca;
    m(0, 1) = -sa;
    m(1, 0) = sa;
    m(1, 1) = ca;
    m(2, 2) = 1.0f;
    m(3, 3) = 1.0f;

    return m;
}

Mat4 Mat4::scale(float factor)
{
    Mat4 m;
    m(0, 0) = factor;
    m(1, 1) = factor;
    m(2, 2) = factor;
    m(3, 3) = 1.0f;

    return m;
}

Mat4 Mat4::scale(Vec3 scale) {
    Mat4 m;
    m(0, 0) = scale.x;
    m(1, 1) = scale.y;
    m(2, 2) = scale.z;
    m(3, 3) = 1.0f;

    return m;
}

std::ostream& operator<<(std::ostream& os, Mat4 const& m)
{
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
            os << m(i, j) << ' ';
        os << std::endl;
    }
    return os;
}