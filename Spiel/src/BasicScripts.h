#pragma once

#include "Script.h"
#include "Engine.h"


class HealthScript : public ScriptController<CompDataHealth, CompController< CompDataHealth>> {
public:
	HealthScript(CompController<CompDataHealth>& cmpCtrl_, Engine& engine_) : ScriptController<CompDataHealth, CompController< CompDataHealth>>(cmpCtrl_, engine_) {}

	inline void executeSample(uint32_t id, CompDataHealth& data, World& world, float deltaTime) override {
		auto [begin, end] = engine.getCollisionInfos(id);
		bool gotHitByBullet{ false };
		for (auto iter = begin; iter != end; ++iter) {
			if (world.bulletCompCtrl.isRegistered(iter->idB)) {
				gotHitByBullet = true;
			}
		}

		if (gotHitByBullet) {
			world.drawableCompCtrl.getComponent(id)->color = vec4(1, 0, 0, 1);
		}
		else {
			world.drawableCompCtrl.getComponent(id)->color = vec4(1, 1, 1, 1);
		}

		if (data.curHealth <= 0) {
			world.despawn(id);
		}
	}
	inline void executeMeta(World& world, float deltaTime) {

	}
};



class AgeScript : public ScriptController<CompDataAge, CompController< CompDataAge>> {
public:
	AgeScript(CompController< CompDataAge>& cmpCtrl_, Engine& engine_) : ScriptController<CompDataAge, CompController< CompDataAge>>(cmpCtrl_, engine_) {}

	inline void executeSample(uint32_t id, CompDataAge& data, World& world, float deltaTime) override {

		data.curAge += deltaTime;

		if (data.curAge > data.maxAge) {
			world.despawn(id);
		}
	}
	inline void executeMeta(World& world, float deltaTime) {

	}
};



class BulletScript : public ScriptController<CompDataBullet, CompController<CompDataBullet>> {
public:
	BulletScript(CompController< CompDataBullet>& cmpCtrl_, Engine& engine_) : ScriptController<CompDataBullet, CompController< CompDataBullet>>(cmpCtrl_, engine_) {}

	inline void executeSample(uint32_t id, CompDataBullet& data, World& world, float deltaTime) override {
		auto [begin, end] = engine.getCollisionInfos(id);
		bool foundCollisionWithMortal{ false };
		for (auto iter = begin; iter != end; ++iter) {
			if (world.healthCompCtrl.isRegistered(iter->idB)) {
				world.healthCompCtrl.getComponent(iter->idB)->curHealth -= data.damage;
				foundCollisionWithMortal = true;
			}
		}

		if (foundCollisionWithMortal) {
			world.despawn(id);
		}
	}
	inline void executeMeta(World& world, float deltaTime) {

	}
};