#include "Mat3.h"

Mat3 Mat3::identity()
{
    Mat3 mat;
    mat(0, 0) = 1; mat(0, 1) = 0; mat(0, 2) = 0;
    mat(1, 0) = 0; mat(1, 1) = 1; mat(1, 2) = 0;
    mat(2, 0) = 0; mat(2, 2) = 0; mat(2, 2) = 1;

    return mat;
}

Mat3 Mat3::translate(Vec2 const& t)
{
    Mat3 mat;
    mat(0, 0) = 1; mat(0, 1) = 0; mat(0, 2) = t.x;
    mat(1, 0) = 0; mat(1, 1) = 1; mat(1, 2) = t.y;
    mat(2, 0) = 0; mat(2, 1) = 0; mat(2, 2) = 1;
    return mat;
}

Mat3 Mat3::scale(float factor)
{
    Mat3 mat;
    mat(0, 0) = factor; mat(0, 1) = 0;       mat(0, 2) = 0;
    mat(1, 0) = 0;       mat(1, 1) = factor; mat(1, 2) = 0;
    mat(2, 0) = 0;       mat(2, 1) = 0;       mat(2, 2) = 1;
    return mat;
}

Mat3 Mat3::scale(Vec2 scale)
{
    Mat3 mat;
    mat(0, 0) = scale.x; mat(0, 1) = 0;       mat(0, 2) = 0;
    mat(1, 0) = 0;       mat(1, 1) = scale.y; mat(1, 2) = 0;
    mat(2, 0) = 0;       mat(2, 1) = 0;       mat(2, 2) = 1;
    return mat;
}

Mat3 Mat3::rotate(float angle)
{
    Mat3 mat;
    mat(0, 0) = cosf(angle / RAD); mat(0, 1) = -sinf(angle / RAD); mat(0, 2) = 0;
    mat(1, 0) = sinf(angle / RAD); mat(1, 1) = cosf(angle / RAD); mat(1, 2) = 0;
    mat(2, 0) = 0;                 mat(2, 1) = 0;                  mat(2, 2) = 1;
    return mat;
}

Mat3 Mat3::rotate(RotaVec2 rotaVec)
{
    Mat3 mat;
    mat(0, 0) = rotaVec.cos; mat(0, 1) = -rotaVec.sin; mat(0, 2) = 0;
    mat(1, 0) = rotaVec.sin; mat(1, 1) =  rotaVec.cos; mat(1, 2) = 0;
    mat(2, 0) = 0;           mat(2, 1) =  0;           mat(2, 2) = 1;
    return mat;
}

Mat3 operator*(Mat3 const& m0, Mat3 const& m1)
{
    Mat3 m;

    for (int i = 0; i < 3; ++i)
    {
        for (int j = 0; j < 3; ++j)
        {
            m(i, j) = 0.0f;
            for (int k = 0; k < 3; ++k)
                m(i, j) += m0(i, k) * m1(k, j);
        }
    }

    return m;
}

Mat3 transpose(Mat3 const& m)
{
    Mat3 mt;
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            mt(i, j) = m(j, i);
    return mt;
}

Mat3 inverse(Mat3 const& m)
{
    float det = (-m(0, 0) * m(1, 1) * m(2, 2)
        + m(0, 0) * m(1, 2) * m(2, 1)
        + m(1, 0) * m(0, 1) * m(2, 2)
        - m(1, 0) * m(0, 2) * m(2, 1)
        - m(2, 0) * m(0, 1) * m(1, 2)
        + m(2, 0) * m(0, 2) * m(1, 1));

    Mat3 inv;
    inv(0, 0) = (m(1, 2) * m(2, 1) - m(1, 1) * m(2, 2)) / det;
    inv(0, 1) = (m(0, 1) * m(2, 2) - m(0, 2) * m(2, 1)) / det;
    inv(0, 2) = (m(0, 2) * m(1, 1) - m(0, 1) * m(1, 2)) / det;
    inv(1, 0) = (m(1, 0) * m(2, 2) - m(1, 2) * m(2, 0)) / det;
    inv(1, 1) = (m(0, 2) * m(2, 0) - m(0, 0) * m(2, 2)) / det;
    inv(1, 2) = (m(0, 0) * m(1, 2) - m(0, 2) * m(1, 0)) / det;
    inv(2, 0) = (m(1, 1) * m(2, 0) - m(1, 0) * m(2, 1)) / det;
    inv(2, 1) = (m(0, 0) * m(2, 1) - m(0, 1) * m(2, 0)) / det;
    inv(2, 2) = (m(0, 1) * m(1, 0) - m(0, 0) * m(1, 1)) / det;

    return inv;
}

std::ostream& operator<<(std::ostream& os, Mat3 const& m)
{
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
            os << m(i, j) << ' ';
        os << std::endl;
    }
    return os;
}