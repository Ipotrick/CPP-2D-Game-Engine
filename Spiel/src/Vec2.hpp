#pragma once

#include <iostream>
#include <boost/serialization/access.hpp>

#include "basic_math.hpp"

class Vec2 {
public:
	Vec2() : x{0}, y{0} {}
	Vec2(float x, float y) : x{ x }, y{ y } {}

	// openGL access func
	inline float const * data() const { return &x; }

	float& operator[](unsigned i) {
		assert(i < 2);
		return (&x)[i];
	}

	float const operator[](unsigned i) const {
		assert(i < 2);
		return (&x)[i];
	}

	bool operator==(Vec2 const& other) const {
		return x == other.x
			&& y == other.y;
	}

	bool operator!=(Vec2 const& other) const {
		return x != other.x
			|| y != other.y;
	}

	Vec2& operator*=(float const scalar) {
		x *= scalar;
		y *= scalar;
		return *this;
	}

	Vec2& operator/=(float const scalar) {
		x /= scalar;
		y /= scalar;
		return *this;
	}

	Vec2& operator*=(Vec2 const& vec) {
		x *= vec.x;
		y *= vec.y;
		return *this;
	}

	Vec2& operator-=(Vec2 const& vec) {
		x -= vec.x;
		y -= vec.y;
		return *this;
	}

	Vec2& operator+=(Vec2 const& vec) {
		x += vec.x;
		y += vec.y;
		return *this;
	}

	float length()
	{
		return sqrtf(x * y);
	}
public:

	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int file_version)
	{
		ar& x;
		ar& y;
	}
	float x;
	float y;
};

inline Vec2 const operator-(Vec2 const& vec) {
	return { -vec.x, -vec.y };
}

inline Vec2 const operator*(float const scalar, Vec2 const& vec) {
	return { scalar * vec.x, scalar * vec.y };
}

inline Vec2 const operator*(const Vec2& vec, const float scalar) {
	return { scalar * vec.x, scalar * vec.y };
}

inline Vec2 const operator*(const Vec2& vecA, const Vec2& vecB) {
	return { vecA.x * vecB.x, vecA.y * vecB.y };
}

inline Vec2 const operator/(Vec2 const& vec, float const scalar) {
	return { vec.x / scalar, vec.y / scalar };
}

inline Vec2 const operator/(Vec2 const& vecA, Vec2 const& vecB) {
	return { vecA.x / vecB.x, vecA.y / vecB.y };
}

inline Vec2 const operator+(Vec2 const& vecA, Vec2 const& vecB) {
	return { vecA.x + vecB.x, vecA.y + vecB.y };
}

inline Vec2 const operator-(Vec2 const& vecA, Vec2 const& vecB) {
	return Vec2(vecA.x - vecB.x, vecA.y - vecB.y);
}

inline Vec2 const min(Vec2 const& vecA, Vec2 const& vecB) {
	return { std::min(vecA.x, vecB.x), std::min(vecA.y, vecB.y) };
}

inline Vec2 const max(Vec2 const& vecA, Vec2 const& vecB) {
	return { std::max(vecA.x, vecB.x), std::max(vecA.y, vecB.y) };
}

// uses trigonometric functions and or sqrt
__forceinline float const norm(Vec2 const& vec) {
	return sqrtf(vec.x * vec.x + vec.y * vec.y);
}

// uses trigonometric functions and or sqrt
inline float const length(Vec2 const& vec) {
	return sqrtf(vec.x * vec.x + vec.y * vec.y);
}

// uses trigonometric functions and or sqrt
inline Vec2 const normalize(Vec2 const& vec) {
	const float n = norm(vec);
	if (n != 0.0)
	{
		return Vec2(vec.x / n,
			vec.y / n);
	}
	return vec;
}

// uses trigonometric functions and or sqrt
__forceinline float const distance(Vec2 const& vecA, Vec2 const& vecB) {
	return norm(vecA - vecB);
}

inline const float dot(Vec2 const& vecA, Vec2 const& vecB) {
	return (vecA.x * vecB.x + vecA.y * vecB.y);
}

inline float const cross(Vec2 const& vecA, Vec2 const& vecB) {
	return vecA.x * vecB.y - vecA.y * vecB.x;
}

inline Vec2 cross(const Vec2& a, float s)
{
	return Vec2(s * a.y, -s * a.x);
}

inline Vec2 cross(float s, const Vec2& a)
{
	return Vec2(-s * a.y, s * a.x);
}

// reflect vector vec at normal n
inline Vec2 const reflect(Vec2 const& vec, Vec2 const& n) {
	return vec - (2.0f * dot(n, vec)) * n;
}

inline std::istream& operator>>(std::istream& is, Vec2& vec) {
	is >> vec.x >> vec.y;
	return is;
}

inline std::ostream& operator<<(std::ostream& os, const Vec2& vec) {
	os << '(' << vec.x << ", " << vec.y << ')';
	return os;
}

// uses trigonometric functions and or sqrt
inline Vec2 rotate(Vec2 const& vec, float angle) {
	float ca = cosf(angle * ((float)M_PI / 180.0f));
	float sa = sinf(angle * ((float)M_PI / 180.0f));
	return { ca * vec.x - sa * vec.y , sa * vec.x + ca * vec.y };
}

// computes sin and cos at compile time
template<auto angle>
inline Vec2 rotate(Vec2 const& vec);

// computes sin and cos at compile time
template<>
inline Vec2 rotate<90>(Vec2 const& vec) {
	return { -vec.y, vec.x };
}

// computes sin and cos at compile time
template<>
inline Vec2 rotate<180>(Vec2 const& vec) {
	return -vec;
}

// computes sin and cos at compile time
template<>
inline Vec2 rotate<270>(Vec2 const& vec) {
	return { -vec.y, -vec.x };
}

// uses trigonometric functions and or sqrt
inline float getRotation(Vec2 vec) {
	if (vec.y == 0.0f)
		return vec.x < 0.0f ? 180.0f : 0.0f;
	else if (vec.x == 0)
		return vec.y < 0.0f ? 270.0f : 90.0f;

	if (vec.y > 0.0f)
		if (vec.x > 0.0f)
			return atan(vec.y / vec.x) * 180.f / 3.14159f;
		else
			return 180.0f - atan(vec.y / -vec.x) * 180.f / 3.14159f;
	else
		if (vec.x > 0)
			return 360.0f - atan(-vec.y / vec.x) * 180.f / 3.14159f;
		else
			return 180.0f + atan(-vec.y / -vec.x) * 180.f / 3.14159f;
}

inline Vec2 abs(Vec2 v)
{
	v.x = fabs(v.x);
	v.y = fabs(v.y);
	return v;
}

class RotaVec2 {
public:
	RotaVec2(float sin, float cos) : sin{ sin }, cos{ cos } {}
	explicit RotaVec2(float angle) : sin{ sinf(angle / RAD) }, cos{ cosf(angle / RAD) } {}

	inline RotaVec2 operator -()
	{
		return RotaVec2(-sin, cos);
	}

	inline bool operator==(RotaVec2 v)
	{
		return this->cos == v.cos && this->sin == v.sin;
	}
public:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int file_version)
	{
		ar& sin;
		ar& cos;
	}
	float sin = 0;
	float cos = 1;
};

inline RotaVec2 operator*(RotaVec2 a, RotaVec2 b)
{
	return RotaVec2(a.sin*b.cos + a.cos*b.sin, a.cos * b.cos - a.sin * b.sin);
}

inline Vec2 rotate(Vec2 vec, RotaVec2 rotationVec) {
	return {
		vec.x * rotationVec.cos - vec.y * rotationVec.sin,
		vec.x * rotationVec.sin + vec.y * rotationVec.cos
	};
}

inline Vec2 rotateInverse(Vec2 vec, RotaVec2 rotationVec) {
	return {
		vec.x * rotationVec.cos + vec.y * rotationVec.sin,
		-vec.x * rotationVec.sin + vec.y * rotationVec.cos
	};
}

inline Vec2 aabbBounds(Vec2 size, RotaVec2 rotaVec) {
	Vec2 maxPos{ 0,0 }; Vec2 minPos{ 0,0 };
	Vec2 point1 = rotate(Vec2( size.x * 0.5f,  size.y * 0.5f), rotaVec);
	Vec2 point2 = rotate(Vec2( size.x * 0.5f, -size.y * 0.5f), rotaVec);
	Vec2 point3 = rotate(Vec2(-size.x * 0.5f,  size.y * 0.5f), rotaVec);
	Vec2 point4 = rotate(Vec2(-size.x * 0.5f, -size.y * 0.5f), rotaVec);

	maxPos = max(max(point1, point2), max(point3, point4));
	minPos = min(min(point1, point2), min(point3, point4));

	return maxPos - minPos;
}
