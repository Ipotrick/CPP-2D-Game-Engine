#include "HealthScript.h"

void HealthScript::script(uint32_t id, Health& data, float deltaTime) {
	World& world = engine.world;
	auto [begin, end] = engine.getCollisions(id);
	bool gotHitByBullet{ false };
	for (auto iter = begin; iter != end; ++iter) {
		if (world.hasComp<Bullet>(iter->idB)) {
			gotHitByBullet = true;
		}
	}

	if (data.curHealth <= 0) {
		world.destroy(id);
	}
}