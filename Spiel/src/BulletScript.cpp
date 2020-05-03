#include "BulletScript.h"

void BulletScript::script(uint32_t id, Bullet& data, float deltaTime) {
	assert(engine.world.doesEntExist(id));
	World& world = engine.world;
	auto [begin, end] = engine.getCollisions(id);
	bool foundCollisionWithMortal{ false };
	for (auto iter = begin; iter != end; ++iter) {
		if (world.hasComp<Health>(iter->idB)) {
			world.getComp<Health>(iter->idB).curHealth -= data.damage;
			foundCollisionWithMortal = true;
		}
	}

	if (foundCollisionWithMortal) {
		if (norm(world.getComp<Collider>(id).size * 0.5f) > 0.02f) {
			auto newEnt = world.createEnt();
			world.addComp<Base>(newEnt, world.getComp<Base>(id));
			world.addComp<Movement>(newEnt, world.getComp<Movement>(id));
			world.getComp<Movement>(newEnt).velocity *= 0.5f;
			world.addComp<Draw>(newEnt, world.getComp<Draw>(id));
			world.getComp<Draw>(newEnt).scale *= 0.5f;
			world.addComp<Collider>(newEnt, world.getComp<Collider>(id));
			world.getComp<Collider>(newEnt).size *= 0.5f;
			Bullet bulletDummy = world.getComp<Bullet>(id);
			bulletDummy.damage /= 2;
			world.addComp<Bullet>(newEnt, bulletDummy);
			world.addComp<PhysicsBody>(newEnt, world.getComp<PhysicsBody>(id));
			world.spawnLater(newEnt);
		}
		world.destroy(id);
	}
}