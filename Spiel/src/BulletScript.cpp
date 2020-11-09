#include "BulletScript.hpp"

void bulletScript(EntityHandle me, Bullet& data, float deltaTime) {
	bool foundHit{ false };
	auto& draw = Engine::world.getComp<Draw>(me);
	draw.color -= Vec4(0,0,0,1) * deltaTime;
	for (CollisionInfo const& collision : Game::collisionSystem.collisions_view(me.index)) {
		if (Engine::world.hasComps<Collider, PhysicsBody>(collision.indexB) && Engine::world.hasntComp<Player>(collision.indexB)) {
			foundHit = true;
		}
		if (Engine::world.hasComp<Health>(collision.indexB)) {
			Engine::world.getComp<Health>(collision.indexB).curHealth -= data.damage;
		}
	}
	if (foundHit == true) {
		data.hitPoints -= 1;
	}
	if (data.hitPoints < 0) {
		Engine::world.despawn(me);
		Engine::world.destroy(me);
	}
}