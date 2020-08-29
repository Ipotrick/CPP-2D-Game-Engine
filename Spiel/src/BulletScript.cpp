#include "BulletScript.hpp"

void BulletScript::script(Entity me, Bullet& data, float deltaTime) {
	assert(engine.world.exists(me));
	World& world = engine.world;
	bool foundHit{ false };
	auto& draw = world.getComp<Draw>(me);
	draw.color -= Vec4(0,0,0,1) * deltaTime;
	for (auto collision : engine.collisionSystem.collisions_view(me)) {
		if (world.hasComps<Collider, PhysicsBody>(collision.indexB) && world.hasntComp<Player>(collision.indexB)) {
			foundHit = true;
		}
		if (world.hasComp<Health>(collision.indexB)) {
			world.getComp<Health>(collision.indexB).curHealth -= data.damage;
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