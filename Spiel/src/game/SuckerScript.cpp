#include "SuckerScript.hpp"

void suckerScript(Game& game, EntityHandle me, SuckerComp& data, float deltaTime)
{
	if (game.world.exists(data.spawner)) {
		auto spawner =game.world.getEntity(data.spawner);
		auto view =game.collisionSystem.collisions_view(me.index);
		for (const auto& collInfo : view) {
			auto ent = collInfo.indexB;
			if (game.world.hasComps<PhysicsBody, Movement>(ent)) {
				auto& spawnerTransform = game.world.getComp<Transform>(spawner);
				game.world.getComp<Transform>(ent).position = spawnerTransform.position + Vec2{rand()%1000/500.0f-1.0f,rand() % 1000 / 500.0f - 1.0f };
			}
		}
	}
}