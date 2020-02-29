#pragma once

#include "math_extention.h"
#include "Basis.h"

class Drawable : virtual public Basis {

public:
	vec2 scale;
	vec3 color;

	Drawable(): color(0.2,0.2,0.2)
	{
		
	}
};