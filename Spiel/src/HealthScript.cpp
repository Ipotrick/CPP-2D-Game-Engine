#include "HealthScript.hpp"

void HealthScript::script(Entity me, Health& data, float deltaTime) {
	World& world = engine.world;
	bool gotHitByBullet{ false };
	for (auto collision : engine.collisionSystem.collisions_view(me)) {
		if (world.hasComp<Bullet>(collision.indexB)) {
			gotHitByBullet = true;
		}
	}

	if (data.curHealth <= 0) {
		world.destroy(me);
	}
}