#include "Game.hpp"

void suckerScript(EntityHandle me, SuckerComp& data, float deltaTime)
{
	if (Game::world.exists(data.spawner)) {
		auto spawner = Game::world.getEntity(data.spawner);
		auto view = Game::collisionSystem.collisions_view(me.index);
		for (const auto& collInfo : view) {
			auto ent = collInfo.indexB;
			if (Game::world.hasComps<PhysicsBody, Movement>(ent)) {
				Game::world.getComp<Transform>(ent).position = Game::world.getComp<Transform>(spawner).position;
			}
		}
	}
}