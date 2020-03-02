#pragma once

#include "glmath.h"
#include "Basis.h"

class Drawable : virtual public Basis {

public:
	vec2 scale;
	vec3 color;

	Drawable():
		Basis{},
		color(1.0f,0.2f,0.2f),
		scale{1,1}
	{
		
	}

	Drawable getDrawable() {
		return *this;
	}
};