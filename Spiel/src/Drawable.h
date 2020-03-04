#pragma once

#include "glmath.h"
#include "Basis.h"

class Drawable : virtual public Basis {

public:
	vec2 scale;
	vec4 color;

	Drawable():
		Basis{},
		scale{ 1, 1 },
		color{ 1.0f,0.2f,0.2f, 1.0f }
	{
		//std::cout << "Drawable Constructor" << std::endl;
		//std::cout << position << std::endl;
	}

	Drawable(vec2 position_, float drawingPrio, vec2 scale_, vec4 color_, float rotation_ = 0.0f) :
		Basis(vec3(position_.x, position_.y, drawingPrio), rotation_),
		scale{ scale_ },
		color{ color_ }
	{
		//std::cout << "Drawable Constructor" << std::endl;
		//std::cout << position << std::endl;
	}

	Drawable getDrawable() {
		return *this;
	}
};