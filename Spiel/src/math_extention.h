#pragma once
#include <initializer_list>
#include <array>
#include <cassert>

/* precision */
#define FLOAT float

class vec2
{
private:

	FLOAT data_[2];

public:

	/// default constructor
	vec2() : data_{ 0,0 } {}

	/// construct with scalar value that is assigned to x, y, and z
	vec2(FLOAT s_) : data_{ s_, s_ } {}

	/// construct with x,y,z values
	vec2(FLOAT x_, FLOAT y_) : data_{ x_, y_ } {}


	/// read/write the _i'th vector component (_i from 0 to 2)
	FLOAT& operator[](unsigned int _i)
	{
		assert(_i < 2);
		return data_[_i];
	}

	/// read the _i'th vector component (_i from 0 to 2)
	const FLOAT operator[](unsigned int _i) const
	{
		assert(_i < 2);
		return data_[_i];
	}

	/// multiply this vector by a scalar \c s
	vec2& operator*=(const FLOAT s)
	{
		for (int i = 0; i < 2; ++i) data_[i] *= s;
		return *this;
	}

	/// divide this vector by a scalar \c s
	vec2& operator/=(const FLOAT s)
	{
		for (int i = 0; i < 2; ++i) data_[i] /= s;
		return *this;
	}

	/// component-wise multiplication of this vector with vector \c v
	vec2& operator*=(const vec2& v)
	{
		for (int i = 0; i < 2; ++i) data_[i] *= v[i];
		return *this;
	}

	/// subtract vector \c v from this vector
	vec2& operator-=(const vec2& v)
	{
		for (int i = 0; i < 2; ++i) data_[i] -= v[i];
		return *this;
	}

	/// add vector \c v to this vector
	vec2& operator+=(const vec2& v)
	{
		for (int i = 0; i < 2; ++i) data_[i] += v[i];
		return *this;
	}
};


//-----------------------------------------------------------------------------


/// unary minus: turn v into -v
inline const vec2 operator-(const vec2& v)
{
	return vec2(-v[0], -v[1]);
}

/// multiply vector \c v by scalar \c s
inline const vec2 operator*(const FLOAT s, const vec2& v)
{
	return vec2(s * v[0],
		s * v[1]);
}

/// multiply vector \c v by scalar \c s
inline const vec2 operator*(const vec2& v, const FLOAT s)
{
	return vec2(s * v[0],
		s * v[1]);
}

/// component-wise multiplication of vectors \c v0 and \c v1
inline const vec2 operator*(const vec2& v0, const vec2& v1)
{
	return vec2(v0[0] * v1[0],
		v0[1] * v1[1]);
}

/// divide vector \c v by scalar \c s
inline const vec2 operator/(const vec2& v, const FLOAT s)
{
	return vec2(v[0] / s,
		v[1] / s);
}

/// add two vectors \c v0 and \c v1
inline const vec2 operator+(const vec2& v0, const vec2& v1)
{
	return vec2(v0[0] + v1[0],
		v0[1] + v1[1]);
}

/// subtract vector \c v1 from vector \c v0
inline const vec2 operator-(const vec2& v0, const vec2& v1)
{
	return vec2(v0[0] - v1[0],
		v0[1] - v1[1]);
}

/// compute the component-wise minimum of vectors \c v0 and \c v1
inline const vec2 min(const vec2& v0, const vec2& v1)
{
	return vec2(std::min(v0[0], v1[0]),
		std::min(v0[1], v1[1]));
}

/// compute the component-wise maximum of vectors \c v0 and \c v1
inline const vec2 max(const vec2& v0, const vec2& v1)
{
	return vec2(std::max(v0[0], v1[0]),
		std::max(v0[1], v1[1]));
}

/// compute the Euclidean dot product of \c v0 and \c v1
inline const FLOAT dot(const vec2& v0, const vec2& v1)
{
	return (v0[0] * v1[0] + v0[1] * v1[1] + v0[2] * v1[2]);
}

/// compute the Euclidean norm (length) of a vector \c v
inline const FLOAT norm(const vec2& v)
{
	return sqrt(dot(v, v));
}

/// compute the Squared Euclidean norm (length) of a vector \c v
inline const FLOAT normSq(const vec2& v)
{
	return dot(v, v);
}

/// normalize vector \c v by dividing it by its norm
inline const vec2 normalize(const vec2& v)
{
	const FLOAT n = norm(v);
	if (n != 0.0)
	{
		return vec2(v[0] / n,
			v[1] / n);
	}
	return v;
}

/// compute the distance between vectors \c v0 and \c v1
inline const FLOAT distance(const vec2& v0, const vec2& v1)
{
	return norm(v0 - v1);
}

/// reflect vector \c v at normal \c n
inline const vec2 reflect(const vec2& v, const vec2& n)
{
	return v - (2.0f * dot(n, v)) * n;
}

/// mirrors vector \c v at normal \c n
inline const vec2 mirror(const vec2& v, const vec2& n)
{
	return (2.0f * dot(n, v)) * n - v;
}

/// read the space-separated components of a vector from a stream
inline std::istream& operator>>(std::istream& is, vec2& v)
{
	is >> v[0] >> v[1];
	return is;
}

/// output a vector by printing its comma-separated compontens
inline std::ostream& operator<<(std::ostream& os, const vec2& v)
{
	os << '(' << v[0] << ", " << v[1] << ')';
	return os;
}

class vec3
{
private:

	FLOAT data_[3];

public:

	/// default constructor
	vec3(): data_{0,0,0} {}

	vec3(vec2 vec2_) { data_[0] = vec2_[0]; data_[1] = vec2_[1]; data_[2] = 0; }

	/// construct with scalar value that is assigned to x, y, and z
	vec3(FLOAT _s) : data_{ _s,_s,_s } {}

	/// construct with x,y,z values
	vec3(FLOAT _x, FLOAT _y, FLOAT _z) : data_{ _x,_y,_z } {}


	/// read/write the _i'th vector component (_i from 0 to 2)
	FLOAT& operator[](unsigned int _i)
	{
		assert(_i < 3);
		return data_[_i];
	}

	/// read the _i'th vector component (_i from 0 to 2)
	const FLOAT operator[](unsigned int _i) const
	{
		assert(_i < 3);
		return data_[_i];
	}


	/// multiply this vector by a scalar \c s
	vec3& operator*=(const FLOAT s)
	{
		for (int i = 0; i < 3; ++i) data_[i] *= s;
		return *this;
	}

	/// divide this vector by a scalar \c s
	vec3& operator/=(const FLOAT s)
	{
		for (int i = 0; i < 3; ++i) data_[i] /= s;
		return *this;
	}

	/// component-wise multiplication of this vector with vector \c v
	vec3& operator*=(const vec3& v)
	{
		for (int i = 0; i < 3; ++i) data_[i] *= v[i];
		return *this;
	}

	/// subtract vector \c v from this vector
	vec3& operator-=(const vec3& v)
	{
		for (int i = 0; i < 3; ++i) data_[i] -= v[i];
		return *this;
	}

	/// add vector \c v to this vector
	vec3& operator+=(const vec3& v)
	{
		for (int i = 0; i < 3; ++i) data_[i] += v[i];
		return *this;
	}
};


//-----------------------------------------------------------------------------


/// unary minus: turn v into -v
inline const vec3 operator-(const vec3& v)
{
	return vec3(-v[0], -v[1], -v[2]);
}

/// multiply vector \c v by scalar \c s
inline const vec3 operator*(const FLOAT s, const vec3& v)
{
	return vec3(s * v[0],
		s * v[1],
		s * v[2]);
}

/// multiply vector \c v by scalar \c s
inline const vec3 operator*(const vec3& v, const FLOAT s)
{
	return vec3(s * v[0],
		s * v[1],
		s * v[2]);
}

/// component-wise multiplication of vectors \c v0 and \c v1
inline const vec3 operator*(const vec3& v0, const vec3& v1)
{
	return vec3(v0[0] * v1[0],
		v0[1] * v1[1],
		v0[2] * v1[2]);
}

/// divide vector \c v by scalar \c s
inline const vec3 operator/(const vec3& v, const FLOAT s)
{
	return vec3(v[0] / s,
		v[1] / s,
		v[2] / s);
}

/// add two vectors \c v0 and \c v1
inline const vec3 operator+(const vec3& v0, const vec3& v1)
{
	return vec3(v0[0] + v1[0],
		v0[1] + v1[1],
		v0[2] + v1[2]);
}

/// subtract vector \c v1 from vector \c v0
inline const vec3 operator-(const vec3& v0, const vec3& v1)
{
	return vec3(v0[0] - v1[0],
		v0[1] - v1[1],
		v0[2] - v1[2]);
}

/// compute the component-wise minimum of vectors \c v0 and \c v1
inline const vec3 min(const vec3& v0, const vec3& v1)
{
	return vec3(std::min(v0[0], v1[0]),
		std::min(v0[1], v1[1]),
		std::min(v0[2], v1[2]));
}

/// compute the component-wise maximum of vectors \c v0 and \c v1
inline const vec3 max(const vec3& v0, const vec3& v1)
{
	return vec3(std::max(v0[0], v1[0]),
		std::max(v0[1], v1[1]),
		std::max(v0[2], v1[2]));
}

/// compute the Euclidean dot product of \c v0 and \c v1
inline const FLOAT dot(const vec3& v0, const vec3& v1)
{
	return (v0[0] * v1[0] + v0[1] * v1[1] + v0[2] * v1[2]);
}

/// compute the Euclidean norm (length) of a vector \c v
inline const FLOAT norm(const vec3& v)
{
	return sqrt(dot(v, v));
}

/// compute the Squared Euclidean norm (length) of a vector \c v
inline const FLOAT normSq(const vec3& v)
{
	return dot(v, v);
}

/// normalize vector \c v by dividing it by its norm
inline const vec3 normalize(const vec3& v)
{
	const FLOAT n = norm(v);
	if (n != 0.0)
	{
		return vec3(v[0] / n,
			v[1] / n,
			v[2] / n);
	}
	return v;
}

/// compute the distance between vectors \c v0 and \c v1
inline const FLOAT distance(const vec3& v0, const vec3& v1)
{
	return norm(v0 - v1);
}

/// compute the cross product of \c v0 and \c v1
inline const vec3 cross(const vec3& v0, const vec3& v1)
{
	return vec3(v0[1] * v1[2] - v0[2] * v1[1],
		v0[2] * v1[0] - v0[0] * v1[2],
		v0[0] * v1[1] - v0[1] * v1[0]);
}

/// reflect vector \c v at normal \c n
inline const vec3 reflect(const vec3& v, const vec3& n)
{
	return v - (2.0f * dot(n, v)) * n;
}

/// mirrors vector \c v at normal \c n
inline const vec3 mirror(const vec3& v, const vec3& n)
{
	return (2.0f * dot(n, v)) * n - v;
}

/// read the space-separated components of a vector from a stream
inline std::istream& operator>>(std::istream& is, vec3& v)
{
	is >> v[0] >> v[1] >> v[2];
	return is;
}

/// output a vector by printing its comma-separated compontens
inline std::ostream& operator<<(std::ostream& os, const vec3& v)
{
	os << '(' << v[0] << ", " << v[1] << ", " << v[2] << ')';
	return os;
}