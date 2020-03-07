#pragma once

#include "glmath.h"


struct Basis {
	/* x y world coordinates, z depth*/
	vec2 position;
	/* in radiants 2pi = one rotation*/
	float rotation;

	Basis() :
		position{ 0.0f, 0.0f },
		rotation{ 0.0f }
	{
	}

	Basis(vec2 position_, float rotation_):
		position{ position_ },
		rotation{ rotation_ }
	{
	}

	inline vec2 getPos() const { return position; }
	inline float getRota() const { return rotation; }
};