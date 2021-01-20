#include "SuckerScript.hpp"

void suckerScript(Game& game, EntityHandle me, SuckerComp& data, float deltaTime)
{
	if (game.world.exists(data.spawner)) {
		auto spawner =game.world.getEntity(data.spawner);
		auto view =game.collisionSystem.collisions_view(me.index);
		for (const auto& collInfo : view) {
			auto ent = collInfo.indexB;
			if (game.world.hasComps<PhysicsBody, Movement>(ent)) {
				game.world.getComp<Transform>(ent).position =game.world.getComp<Transform>(spawner).position;
			}
		}
	}
}