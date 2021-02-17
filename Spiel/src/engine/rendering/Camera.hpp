#pragma once

#include "../math/vector_math.hpp"

struct Camera {
	Vec2 position{ 0,0 };
	Vec2 frustumBend{ 1,1 };
	float zoom{ 1 };
	float rotation{ 0 };
};