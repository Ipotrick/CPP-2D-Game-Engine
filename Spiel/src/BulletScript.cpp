#include "BulletScript.h"

void BulletScript::script(entity_id me, Bullet& data, float deltaTime) {
	assert(engine.world.exists(me));
	World& world = engine.world;
	auto [begin, end] = engine.getCollisions(me);
	bool foundHit{ false };
	for (auto iter = begin; iter != end; ++iter) {
		if (world.hasComps<Collider, PhysicsBody>(iter->idB) && world.hasntComp<Player>(iter->idB)) {
			foundHit = true;
		}
		if (world.hasComp<Health>(iter->idB)) {
			world.getComp<Health>(iter->idB).curHealth -= data.damage;
		}
	}
	if (foundHit == true) {
		data.hitPoints -= 1;
	}
	if (data.hitPoints < 0) {
		world.despawn(me);
		world.destroy(me);
	}
}