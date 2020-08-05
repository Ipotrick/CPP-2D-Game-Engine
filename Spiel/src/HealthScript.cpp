#include "HealthScript.hpp"

void HealthScript::script(Entity id, Health& data, float deltaTime) {
	World& world = engine.world;
	auto [begin, end] = engine.getCollisions(id);
	bool gotHitByBullet{ false };
	for (auto iter = begin; iter != end; ++iter) {
		if (world.hasComp<Bullet>(iter->indexB)) {
			gotHitByBullet = true;
		}
	}

	if (data.curHealth <= 0) {
		world.destroy(id);
	}
}