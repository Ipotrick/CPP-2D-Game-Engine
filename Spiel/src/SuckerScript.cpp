#include "SuckerScript.hpp"

void SuckerScript::script(Entity me, SuckerComp& data, float deltaTime) {
	World& world = engine.world;
	if (world.isIdValid(data.spawner)) {
		for (auto collInfo : engine.collisions_view(me)) {
			auto ent = collInfo.indexB;
			if (world.hasComps<PhysicsBody, Movement>(ent)) {
				world.getComp<Base>(ent).position = world.getComp<Base>(data.spawner).position;
			}
		}
	}
}