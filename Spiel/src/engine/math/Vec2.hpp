#pragma once

#include "../types/ShortNames.hpp"

#include <iostream>

#include "basic_math.hpp"

class Vec2 {
public:
	constexpr Vec2() {};

	constexpr Vec2(f32 x, f32 y) : 
		x{ x }, y{ y } 
	{}

	constexpr f32 const* data() const { return &x; }

	constexpr f32& operator[](u32 index)
	{
		assert(index < 2);
		return (&x)[index];
	}
	
	constexpr const f32& operator[](u32 index) const
	{
		assert(index < 2);
		return (&x)[index];
	}
public:
	f32 x{ 0.0f };
	f32 y{ 0.0f };
};


inline constexpr Vec2& operator*=(Vec2& vec, f32 scalar)
{
	vec.x *= scalar;
	vec.y *= scalar;
	return vec;
}

inline constexpr Vec2& operator/=(Vec2& vec, f32 scalar)
{
	vec.x /= scalar;
	vec.y /= scalar;
	return vec;
}

inline constexpr Vec2& operator*=(Vec2& vecA, Vec2 vecB)
{
	vecA.x *= vecB.x;
	vecA.y *= vecB.y;
	return vecA;
}

inline constexpr Vec2& operator-=(Vec2& vecA, Vec2 vecB)
{
	vecA.x -= vecB.x;
	vecA.y -= vecB.y;
	return vecA;
}

inline constexpr Vec2& operator+=(Vec2& vecA, Vec2 vecB)
{
	vecA.x += vecB.x;
	vecA.y += vecB.y;
	return vecA;
}
inline constexpr bool operator==(Vec2 vecA, Vec2 vecB)
{
	return vecA.x == vecB.x & vecA.y == vecB.y;
}

inline constexpr bool operator!=(Vec2 vecA, Vec2 vecB)
{
	return vecA.x != vecB.x | vecA.y != vecB.y;
}

inline constexpr Vec2 round(Vec2 vec)
{
	return Vec2{ std::round(vec.x), std::round(vec.y) };
}

inline constexpr Vec2 floor(Vec2 vec)
{
	return Vec2{ std::floor(vec.x), std::floor(vec.y) };
}

inline constexpr Vec2 ceil(Vec2 vec)
{
	return Vec2{ std::ceil(vec.x), std::ceil(vec.y) };
}

inline constexpr Vec2 operator-(Vec2 vec) {
	return { -vec.x, -vec.y };
}

inline constexpr Vec2 operator*(f32 scalar, Vec2 vec) {
	return { scalar * vec.x, scalar * vec.y };
}

inline constexpr Vec2 operator*(Vec2 vec, f32 scalar) {
	return { scalar * vec.x, scalar * vec.y };
}

inline constexpr Vec2 operator*(Vec2 vecA, Vec2 vecB) {
	return { vecA.x * vecB.x, vecA.y * vecB.y };
}

inline constexpr Vec2 operator/(Vec2 vec, f32 scalar) {
	return { vec.x / scalar, vec.y / scalar };
}

inline constexpr Vec2 operator/(Vec2 vecA, Vec2 vecB) {
	return { vecA.x / vecB.x, vecA.y / vecB.y };
}

inline constexpr Vec2 operator+(Vec2 vecA, Vec2 vecB) {
	return { vecA.x + vecB.x, vecA.y + vecB.y };
}

inline constexpr Vec2 operator-(Vec2 vecA, Vec2 vecB) {
	return Vec2(vecA.x - vecB.x, vecA.y - vecB.y);
}

inline constexpr Vec2 min(Vec2 vecA, Vec2 vecB) {
	return { std::min(vecA.x, vecB.x), std::min(vecA.y, vecB.y) };
}

inline constexpr Vec2 max(Vec2 vecA, Vec2 vecB) {
	return { std::max(vecA.x, vecB.x), std::max(vecA.y, vecB.y) };
}

inline f32 length(Vec2 vec)
{
	return sqrtf(vec.x * vec.x + vec.y * vec.y);
}

inline Vec2 normalize(Vec2 vec)
{
	// assert(vec.x != 0.0f || vec.y != 0.0f);
	f32 invLen = 1.0f / length(vec);
	return { vec.x * invLen, vec.y * invLen };
}

inline f32 distance(Vec2 vecA, Vec2 vecB)
{
	return length(vecA - vecB);
}

inline constexpr f32 dot(Vec2 vecA, Vec2 vecB) {
	return (vecA.x * vecB.x + vecA.y * vecB.y);
}

inline constexpr f32 cross(Vec2 vecA, Vec2 vecB) {
	return vecA.x * vecB.y - vecA.y * vecB.x;
}

inline constexpr Vec2 cross(Vec2 vec, f32 scalar)
{
	return Vec2(scalar * vec.y, -scalar * vec.x);
}

inline constexpr Vec2 cross(f32 scalar, Vec2 vec)
{
	return Vec2(-scalar * vec.y, scalar * vec.x);
}

inline constexpr Vec2 reflect(Vec2 vec, Vec2 n) {
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

inline Vec2 rotate(Vec2 vec, f32 angle) {
	f32 ca = cosf(angle * ((f32)M_PI / 180.0f));
	f32 sa = sinf(angle * ((f32)M_PI / 180.0f));
	return { ca * vec.x - sa * vec.y , sa * vec.x + ca * vec.y };
}

template<u32 angle>
inline Vec2 rotate(Vec2 vec);

template<>
inline Vec2 rotate<90>(Vec2 vec) {
	return { -vec.y, vec.x };
}

template<>
inline Vec2 rotate<180>(Vec2 vec) {
	return -vec;
}

template<>
inline Vec2 rotate<270>(Vec2 vec) {
	return { -vec.y, -vec.x };
}

inline f32 angle(Vec2 vec) {
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

inline constexpr Vec2 abs(Vec2 vec)
{
	return { fabs(vec.x), fabs(vec.y) };
}

/*
	TODO MASSIVE REWORK
*/
class RotaVec2 {
public:
	RotaVec2() = default;
	RotaVec2(f32 sin, f32 cos) : sin{ sin }, cos{ cos } {}
	explicit RotaVec2(f32 angle) : sin{ sinf(angle / RAD) }, cos{ cosf(angle / RAD) } {}

	static RotaVec2 fromUnitX0(Vec2 vec)
	{
		return RotaVec2{ vec.y, vec.x };
	}
	static RotaVec2 fromUnitY0(Vec2 vec)
	{
		return RotaVec2{ vec.x, -vec.y };
	}

	RotaVec2 operator -()
	{
		return RotaVec2(cos , -sin);
	}

	bool operator==(RotaVec2 v)
	{
		return this->cos == v.cos && this->sin == v.sin;
	}

	/**
	 * \return unit length vector that is rotated by the rotavec, where y=1 up is the 0 rotation
	 */
	Vec2 toUnitX0() const
	{
		return { cos, sin };
	}

	/**
	 * \return unit length vector that is rotated by the rotavec, where x=1 up is the 0 rotation
	 */
	Vec2 toUnitY0() const
	{
		return { -sin, cos };
	}
public:
	f32 cos{ 1.0f };
	f32 sin{ 0.0f };
};

inline RotaVec2 operator*(RotaVec2 a, RotaVec2 b)
{
	return RotaVec2(a.sin*b.cos + a.cos*b.sin, a.cos * b.cos - a.sin * b.sin);
}

inline RotaVec2& operator*=(RotaVec2& a, RotaVec2 b)
{
	auto c = a * b;
	a = c;
	return a;
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

inline constexpr bool isPointInRange(Vec2 p, Vec2 min, Vec2 max)
{
	return
		p.x <= max.x &
		p.y <= max.y &
		p.x >= min.x &
		p.y >= min.y;
}

inline constexpr bool isPointInAABB(Vec2 p, Vec2 aabbCenter, Vec2 aabbSize)
{
	const Vec2 relativePointPos = p - aabbCenter;
	aabbSize *= 0.5f;
	return
		relativePointPos.x <= aabbSize.x &
		relativePointPos.y <= aabbSize.y &
		relativePointPos.x >= -aabbSize.x &
		relativePointPos.y >= -aabbSize.y;
}

template<>
inline Vec2 clamp<Vec2>(const Vec2 vec, const Vec2 min, const Vec2 max)
{
	return ::max(min, ::min(max, vec));
}