#include "SuckerScript.hpp"

void SuckerScript::script(Entity me, SuckerComp& data, float deltaTime) {
	World& world = engine.world;
	if (world.isIdValid(data.spawner)) {
		auto view = engine.collisionSystem.collisions_view(me);
		for (auto collInfo : view) {
			auto ent = collInfo.indexB;
			if (world.hasComps<PhysicsBody, Movement>(ent)) {
				world.getComp<Base>(ent).position = world.getComp<Base>(data.spawner).position;
			}
		}
	}
}