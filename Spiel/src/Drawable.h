#pragma once

#include "math_extention.h"
#include "Basis.h"

class Drawable : virtual public Basis {

public:
	vec2 scale;
	vec3 color;

	Drawable():
		Basis{},
		color(0.2f,0.2f,0.2f)
	{
		
	}

	Drawable getDrawable() {
		return *this;
	}
};