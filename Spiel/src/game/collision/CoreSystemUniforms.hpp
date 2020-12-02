#pragma once

#include "../../engine/Vec2.hpp"

struct PhysicsUniforms {
	float friction{ 0 };
	Vec2  linearEffectDir{ 0, 0 };
	float linearEffectAccel{ 0 };
	float linearEffectForce{ 0 };
private:
};