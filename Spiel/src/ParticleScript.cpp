#include "ParticleScript.hpp"

void ParticleScript::script(Entity me, ParticleScriptComp& data, float deltaTime) {

	//auto[age, draw, mov] = world.getComps<Age, Draw, Movement>(me);
	auto& age = world.getComp<Age>(me);
	auto& draw = world.getComp<Draw>(me);
	auto& mov = world.getComp<Movement>(me);

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

	mov.velocity.x += (rand() % 1000 / 400.0f - 1.25f) * deltaTime;
	mov.velocity.y += (rand() % 1000 / 400.0f - 1.25f) * deltaTime;
	mov.velocity *= 1 - deltaTime;
	mov.angleVelocity *= 1 - deltaTime*8;

	if (world.hasComp<Collider>(me)) {
		auto& collider = world.getComp<Collider>(me);
		bool coll = false;
		for (const auto collision : engine.collisionSystem.collisions_view(me)) {
			if (world.hasComps<Collider, PhysicsBody>(collision.indexB)) {
				coll = true;
			}
		}
		if (coll) {
			data.startSize *= 0.7f;
			data.endSize *= 0.8f;
			data.endColor.a *= 0.5f;
			age.curAge += 0.01f;
			mov.velocity *= 0.8f;
			data.collisionCount++;
		}

		if (relativeAge > 0.1f) {
			collider.ignoreGroupMask = 0;
		}

		collider.size = logInterpolation(data.startSize, data.endSize);


		if (relativeAge > 0.7f || data.collisionCount > 2) {
			world.remComp<Collider>(me);
			mov.velocity = Vec2(0, 0);
			mov.angleVelocity *= 0.5f;
		}
	}

	draw.scale = logInterpolation(data.startSize, data.endSize);
	draw.color = logInterpolation(data.startColor, data.endColor);

	age.curAge += rand() % 1000 / 2000 * deltaTime;
}