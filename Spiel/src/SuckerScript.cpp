#include "Game.hpp"

void suckerScript(EntityHandle me, SuckerComp& data, float deltaTime)
{
	if (Engine::world.exists(data.spawner)) {
		auto spawner = Engine::world.getEntity(data.spawner);
		auto view = Game::collisionSystem.collisions_view(me.index);
		for (const auto& collInfo : view) {
			auto ent = collInfo.indexB;
			if (Engine::world.hasComps<PhysicsBody, Movement>(ent)) {
				Engine::world.getComp<Transform>(ent).position = Engine::world.getComp<Transform>(spawner).position;
			}
		}
	}
}