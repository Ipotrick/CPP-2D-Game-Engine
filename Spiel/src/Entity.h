#pragma once

#include <algorithm> 
#include <atomic>

#include "glmath.h"

enum class Form : uint8_t{
	CIRCLE = 0x00,
	RECTANGLE = 0x01
};

inline std::ostream& operator<<(std::ostream& ios, Form form) {
	ios << ((bool)form ? "Rectangle" : "Circle");
	return ios;
}

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

	Basis(vec2 position_, float rotation_) :
		position{ position_ },
		rotation{ rotation_ }
	{
	}

	inline vec2 getPos() const { return position; }
	inline float getRota() const { return rotation; }
};



class Drawable : virtual public Basis {
public:
	vec4 color;
	vec2 scale;
	float drawingPrio;
	uint32_t id;
	Form form;
	bool throwsShadow;

	Drawable(uint32_t id_, vec2 position_, float drawingPrio_, vec2 scale_, vec4 color_, Form form_, float rotation_, bool throwsShadow_ = false) :
		Basis(vec2(position_.x, position_.y), rotation_), 
		drawingPrio{ drawingPrio_ },
		scale{ scale_ },
		color{ color_ },
		id{ id_ },
		form{ form_ },
		throwsShadow{ throwsShadow_ }
	{
	}
};