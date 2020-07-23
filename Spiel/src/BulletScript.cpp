#include "BulletScript.hpp"

void BulletScript::script(entity_id me, Bullet& data, float deltaTime) {
	assert(engine.world.exists(me));
	World& world = engine.world;
	auto [begin, end] = engine.getCollisions(me);
	bool foundHit{ false };
	auto& draw = world.getComp<Draw>(me);
	draw.color -= Vec4(0,0,0,1) * deltaTime;
	for (auto iter = begin; iter != end; ++iter) {
		if (world.hasComps<Collider, PhysicsBody>(iter->indexB) && world.hasntComp<Player>(iter->indexB)) {
			foundHit = true;
		}
		if (world.hasComp<Health>(iter->indexB)) {
			world.getComp<Health>(iter->indexB).curHealth -= data.damage;
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