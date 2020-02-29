#pragma once

#include "math_extention.h"

struct Basis {
	/* x y world coordinates, z depth*/
	vec3 position;
	/* in radiants 2pi = one rotation*/
	float rotation;

	inline void setPos(vec3 pos_) { position = pos_; }
	inline void setPos(vec2 pos_) { position[0] = pos_[0]; position[1] = pos_[1]; }
};