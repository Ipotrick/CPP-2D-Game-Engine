#pragma once

#include "glmath.h"

struct Basis {
	/* x y world coordinates, z depth*/
	vec3 position;
	/* in radiants 2pi = one rotation*/
	float rotation;

	Basis() :
		position{ vec3(0.0f, 0.0f, 0.0f) },
		rotation{ 0.0f }
	{
		//std::cout << "Basis Constructor" << std::endl;
		//std::cout << position << std::endl;
	}

	Basis(vec3 position_, float rotation_):
		position{ position_ },
		rotation{ rotation_ }
	{
		//std::cout << "Basis Constructor" << std::endl;
		//std::cout << position << std::endl;
	}
};