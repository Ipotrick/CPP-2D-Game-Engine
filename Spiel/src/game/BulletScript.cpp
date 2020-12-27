#include "BulletScript.hpp"

void bulletScript(EntityHandle me, Bullet& data, float deltaTime) {
	auto& draw = Game::world.getComp<Draw>(me);
	Age& age = Game::world.getComp<Age>(me);
	draw.color.w = 1.0f - age.curAge / age.maxAge;

	bool foundHit{ false };
	for (CollisionInfo const& collision : Game::collisionSystem.collisions_view(me.index)) {
		if (Game::world.hasComps<Collider, PhysicsBody>(collision.indexB) && Game::world.hasntComp<Player>(collision.indexB)) {
			foundHit = true;
		}
		if (Game::world.hasComp<Health>(collision.indexB)) {
			Game::world.getComp<Health>(collision.indexB).curHealth -= data.damage;
		}
	}
	if (foundHit == true) {
		// data.hitPoints -= 1;
		data.hitPoints = 0;
	}
	if (data.hitPoints <= 0) {
		Game::world.despawn(me);
		Game::world.destroy(me);
	}
}