#include "ParticleScript.hpp"

void ParticleScript::script(entity_id entity, ParticleScriptComp& data, float deltaTime) {
	assert(world.hasComp<Age>(entity));
	auto& age = world.getComp<Age>(entity);
	assert(world.hasComp<Draw>(entity)); 
	auto& draw = world.getComp<Draw>(entity);
	assert(world.hasComp<Collider>(entity));
	auto& collider = world.getComp<Collider>(entity);
 
	auto [begin, end] = engine.getCollisions(entity);
	for (auto iter = begin; iter != end; ++iter) {
		if (world.hasComps<Collider, PhysicsBody>(iter->indexB)) {
			data.startSize *= 0.5f;
			data.endSize *= 0.9f;
		}
	}

	draw.scale = data.startSize * (1 - age.curAge / age.maxAge) + data.endSize * age.curAge / age.maxAge;
	collider.size = data.startSize * 0.6 * (1 - age.curAge / age.maxAge) + data.endSize * age.curAge / age.maxAge;
	draw.color = data.startColor * (1 - age.curAge / age.maxAge) + data.endColor * age.curAge / age.maxAge;
}