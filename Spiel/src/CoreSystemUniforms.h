#pragma once

#include "Vec2.h"

struct SystemUniformsPhysics {
	float friction{ 0 };
	Vec2  linearEffectDir{ 0 };
	float linearEffectAccel{ 0 };
	float linearEffectForce{ 0 };
};