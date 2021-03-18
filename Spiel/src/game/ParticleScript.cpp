#include "ParticleScript.hpp"

void particleScript(Game& game, EntityHandle me, ParticleScriptComp& data, float deltaTime)
{
	auto& world = game.world;
	//auto[age, draw, mov] = world.getComps<Age, Draw, Movement>(me);
	auto& age = world.getComp<Age>(me);
	auto& draw = world.getComp<Draw>(me);
	auto& mov = world.getComp<Movement>(me);

	float relativeAge = age.curAge / age.maxAge;

	auto linearInterpolation = [relativeAge](auto start, auto end) {
		return start * (1.0f - relativeAge) + end * relativeAge;
	};

	auto quadraticInterpolation = [relativeAge](auto start, auto end) {
		auto qAge = relativeAge * relativeAge;
		return start * (1.0f - qAge) + end * qAge;
	};
	int str = 2;
	auto logInterpolation = [relativeAge, str](auto start, auto end) {
		float logAge = logf(1.71f * relativeAge + 1.0f);
		for (int i = 1; i < str; i++)
			logAge = logf(1.71f * logAge + 1.0f);
		return start * (1 - logAge) + end * logAge;
	};

	mov.velocity.x += (rand() % 1000 / 400.0f - 1.25f) * deltaTime;
	mov.velocity.y += (rand() % 1000 / 400.0f - 1.25f) * deltaTime;
	mov.velocity *= 1 - deltaTime;
	mov.angleVelocity *= 1 - deltaTime*8;
	
	if (world.hasComp<Collider>(me)) {
		auto& collider = world.getComp<Collider>(me);
		bool coll = false;
		auto collisionView = game.collisionSystem.collisions_view(me);
		for (const auto collision : collisionView) {
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