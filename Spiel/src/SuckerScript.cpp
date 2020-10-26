#include "SuckerScript.hpp"

void SuckerScript::script(EntityHandle me, SuckerComp& data, float deltaTime) {
	World& world = engine.world;
	if (world.isHandleValid(data.spawner)) {
		auto view = engine.collisionSystem.collisions_view(me.index);
		for (auto collInfo : view) {
			auto ent = collInfo.indexB;
			if (world.hasComps<PhysicsBody, Movement>(ent)) {
				world.getComp<Transform>(ent).position = world.getComp<Transform>(data.spawner).position;
			}
		}
	}
}