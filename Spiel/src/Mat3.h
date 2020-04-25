#pragma once

#include "Vec.h"

class Mat3 {
public:

	Mat3() : data_{ 0, 0, 0, 0, 0, 0, 0, 0, 0 } {}

	/// read/write access to element (i,j)
	float& operator()(int const i, int const j) { return data_[i + j * 3]; }

	/// read access to element (i,j)
	float operator()(int const i, int const j) const { return data_[i + j * 3]; }


	/// return identity matrix
	static Mat3 identity();
	/// return translation matrix
	static Mat3 translate(Vec2 const& t);
	// return scaling matrix
	static Mat3 scale(float factor);
	// return scaling matrix
	static Mat3 scale(Vec2 scale);
	/// return matrix for rotation
	static Mat3 rotate(float angle);
	/// return matrix for rotation
	static Mat3 rotate(RotaVec2 rotaVec);

	/// pointer to data (for passing it to OpenGL)
	float const* data() const { return data_; }
private:
	float data_[9];
};


//-----------------------------------------------------------------------------


/// return matrix product matA*matB
Mat3 operator*(Mat3 const& matA, Mat3 const& matB);


/// return matrix-vector product mat*v
inline Vec3 operator*(Mat3 const& mat, const Vec3& vec) {
	Vec3 v;

	for (int i = 0; i < 3; ++i)
	{
		v[i] = 0.0;
		for (int j = 0; j < 3; ++j)
		{
			v[i] += mat(i, j) * vec[j];
		}
	}

	return v;
}

/// return matrix-vector product mat*v
inline Vec2 operator*(Mat3 const& mat, Vec2 const& vec) {
	Vec2 v;

	for (int i = 0; i < 2; ++i)
	{
		v[i] = 0.0;
		for (int j = 0; j < 2; ++j)
		{
			v[i] += mat(i, j) * vec[j];
		}
		v[i] += mat(i, 2) * 1;
	}

	return v;
}

/// return transposed matrix
Mat3 transpose(Mat3 const& mat);

/// return inverse of 3x3 matrix
Mat3 inverse(Mat3 const& mat);

/// print matrix to output stream
std::ostream& operator<<(std::ostream& os, Mat3 const& mat);