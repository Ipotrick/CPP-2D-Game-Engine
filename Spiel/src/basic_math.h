#pragma once

#include <assert.h>
#include <cmath>
#include <algorithm>

#ifdef _WIN32
#define M_PI 3.14159265f
#define RAD 57.2957795f
#endif

inline constexpr float deg2rad(const float deg) { return deg / RAD; }
inline constexpr float rad2deg(const float rad) { return rad * RAD; }