#pragma once

#include "vector_math.hpp"

/* Camera */
class Camera {
public:
	Camera() :
		position{ 0.0f, 0.0f },
		zoom{ 1.0f },
		rotation{ 0.0f },
		frustumBend{ Vec2(1.0f, 1.0f) }
	{}
	Vec2 position;
	float zoom;
	float rotation;
	Vec2 frustumBend;
};