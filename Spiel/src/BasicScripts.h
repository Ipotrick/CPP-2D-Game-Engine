#pragma once

#include "Script.h"
#include "Engine.h"


class HealthScript : public ScriptController<Health> {
public:
	HealthScript(Engine& engine_) : ScriptController<Health>(engine_) {}

	inline void executeSample(uint32_t id, Health& data, float deltaTime) override {
		World& world = engine.world;
		auto [begin, end] = engine.getCollisionInfos(id);
		bool gotHitByBullet{ false };
		for (auto iter = begin; iter != end; ++iter) {
			if (world.hasComp<Bullet>(iter->idB)) {
				gotHitByBullet = true;
			}
		}

		if (data.curHealth <= 0) {
			world.despawn(id);
		}
	}

	inline void executeMeta(float deltaTime) override {

	}
};



class AgeScript : public ScriptController<Age> {
public:
	AgeScript(Engine& engine_) : ScriptController<Age>(engine_) {}

	inline void executeSample(uint32_t id, Age& data, float deltaTime) override {

		data.curAge += deltaTime;

		if (data.curAge > data.maxAge) {
			engine.world.despawn(id);
		}
	}
	inline void executeMeta(float deltaTime) override {

	}
};



class BulletScript : public ScriptController<Bullet> {
public:
	BulletScript(Engine& engine_) : ScriptController<Bullet>(engine_) {}

	inline void executeSample(uint32_t id, Bullet& data, float deltaTime) override {
		assert(engine.world.doesEntExist(id));
		World& world = engine.world;
		auto [begin, end] = engine.getCollisionInfos(id);
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
				bulletDummy.damage *= 0.5f;
				delayedAddComp.push_back({ newEnt, bulletDummy });
				world.addComp<SolidBody>(newEnt, world.getComp<SolidBody>(id));
			}
			world.despawn(id);
		}
	}
	inline void executeMeta(float deltaTime) override {

	}
};