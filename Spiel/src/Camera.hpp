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

	Vec2 windowToWorld(const Vec2 windowSpacePosition) const
	{
		return Mat3::translate(position) * Mat3::rotate(rotation) * Mat3::scale(Vec2(1 / frustumBend.x, 1 / frustumBend.y)) * Mat3::scale(1 / zoom) * Vec2(windowSpacePosition.x, windowSpacePosition.y);
	}


	Vec2 position;
	float zoom;
	float rotation;
	Vec2 frustumBend;
};