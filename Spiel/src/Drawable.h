#pragma once

#include "glmath.h"
#include "Basis.h"

class Drawable : virtual public Basis {

public:
	enum class Form {
		CIRCLE,
		RECTANGLE
	};
	vec4 color;
	vec2 scale;
	float drawingPrio;
	Form form;

	Drawable() :
		Basis{},
		drawingPrio{ 0.0f },
		scale{ 1, 1 },
		color{ 1.0f,0.2f,0.2f, 1.0f },
		form{ Form::RECTANGLE }
	{
		//std::cout << "Drawable Constructor" << std::endl;
		//std::cout << position << std::endl;
	}

	Drawable(vec2 position_, float drawingPrio_, vec2 scale_, vec4 color_, Form form_ = Form::RECTANGLE, float rotation_ = 0.0f) :
		Basis(vec2(position_.x, position_.y), rotation_),
		drawingPrio{ drawingPrio_ },
		scale{ scale_ },
		color{ color_ },
		form{ form_ }
	{
		//std::cout << "Drawable Constructor" << std::endl;
		//std::cout << position << std::endl;
	}

	Drawable getDrawable() const {
		return *this;
	}
};