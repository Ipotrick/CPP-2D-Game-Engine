#include "ParticleScript.hpp"

void ParticleScript::script(Entity me, ParticleScriptComp& data, float deltaTime) {
	assert(world.hasComp<Age>(me));
	auto& age = world.getComp<Age>(me);
	assert(world.hasComp<Draw>(me)); 
	auto& draw = world.getComp<Draw>(me);
	assert(world.hasComp<Collider>(me));
	auto& collider = world.getComp<Collider>(me);
	auto mov = world.getComp<Movement>(me);

	mov.angleVelocity += (rand() % 1000 / 400.0f * 90.0f - 45.0f) * deltaTime;
	mov.velocity.x += (rand() % 1000 / 400.0f - 1.25f) * deltaTime;
	mov.velocity.y += (rand() % 1000 / 400.0f - 1.25f) * deltaTime;
 
	for (const auto collision : engine.collisionSystem.collisions_view(me)) {
		if (world.hasComps<Collider, PhysicsBody>(collision.indexB)) {
			data.startSize *= 0.5f;
			data.endSize *= 0.9f;
			data.endColor.a *= 0.5f;
		}
	}

	if (age.curAge / age.maxAge > 0.05f) {
		collider.ignoreGroupMask = 0;
	}


	if (age.curAge / age.maxAge > 0.7f) {
		collider.ignoreGroupMask = 0xFFFFFFFF;
	}

	float relativeAge = age.curAge / age.maxAge;

	auto linearInterpolation = [relativeAge](auto start, auto end) {
		return start * (1 - relativeAge) + end * relativeAge;
	};

	auto quadraticInterpolation = [relativeAge](auto start, auto end) {
		auto qAge = relativeAge * relativeAge;
		return start * (1 - qAge) + end * qAge;
	};
	int str = 2;
	auto logInterpolation = [relativeAge, str](auto start, auto end) {
		auto logAge = log(1.71 * relativeAge + 1); 
		for (int i = 1; i < str; i++)
			logAge = log(1.71 * logAge + 1);
		return start * (1 - logAge) + end * logAge;
	};

	draw.scale = logInterpolation(data.startSize, data.endSize);
	collider.size = logInterpolation(data.startSize, data.endSize);
	draw.color = logInterpolation(data.startColor, data.endColor);

	age.curAge += rand() % 1000 / 1000 * deltaTime;
}