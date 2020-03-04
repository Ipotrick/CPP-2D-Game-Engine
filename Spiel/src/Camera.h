#pragma once

#include "glmath.h"

/* Camera */
class Camera {
public:
	Camera() :
		position{ 0.0f, 0.0f },
		zoom{ 1.0f },
		rotation{ 0.0f },
		frustumBend{ vec2(1.0f, 1.0f) }
	{}
	vec2 position;
	float zoom;
	float rotation;
	vec2 frustumBend;
};