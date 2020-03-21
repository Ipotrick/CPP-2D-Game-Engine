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
				world.healthCompCtrl.getComponentPtr(iter->idB)->curHealth -= data.damage;
				foundCollisionWithMortal = true;
			}
		}

		if (foundCollisionWithMortal) {
			world.despawn(id);
			auto newEnt = world.getEntityPtr(id);
			newEnt->size *= 0.5f;
			if (norm(newEnt->size) > 0.02f) {
				newEnt->velocity *= 0.5f;
				auto newDrawable = world.drawableCompCtrl.getComponentPtr(id);
				newDrawable->scale *= 0.5f;
				world.spawnSolidEntity(*newEnt, *newDrawable, world.solidBodyCompCtrl.getComponent(id));
				world.bulletCompCtrl.registerEntity(world.getLastID(), CompDataBullet(data.damage * 0.5f));
			}
		}
	}
	inline void executeMeta(World& world, float deltaTime) {

	}
};


class TriggerScript : public ScriptController<CompDataTrigger, CompController<CompDataTrigger>> {
public:
	TriggerScript(CompController< CompDataTrigger>& cmpCtrl_, Engine& engine_) : ScriptController<CompDataTrigger, CompController< CompDataTrigger>>(cmpCtrl_, engine_), spawnTimer{ 0.001 } {}
	LapTimer<> spawnTimer;


	inline void executeSample(uint32_t id, CompDataTrigger& data, World& world, float deltaTime) override {
		auto begin = world.playerCompCtrl.componentData.begin();
		auto end = world.playerCompCtrl.componentData.end();
		for (auto iter = begin; iter != end; ++iter)
		{
			auto [begin2, end2] = engine.getCollisionInfos(id);
			{
				for (auto iter2 = begin2; iter2 != end2; ++iter2)
				{
					if (iter->first == iter2->idB)
					{
						vec2 scale = vec2(0.05f, 0.05f);
						Entity trashEntC = Entity(vec2(0, 0), 0.0f, Collidable(scale, Form::CIRCLE, true, vec2(0, 0)));
						CompDataDrawable trashEntD = CompDataDrawable(vec4(1, 1, 1, 1), scale, 0.5f, Form::CIRCLE, true);

						for (int i = 0; i < spawnTimer.getLaps(deltaTime); i++) {
							world.spawnSolidEntity(trashEntC,trashEntD, CompDataSolidBody(0.9f, 0.5f));
							world.healthCompCtrl.registerEntity(world.getLastID(), CompDataHealth(100));
						}
					}
				}
			}
		}
	}
	inline void executeMeta(World& world, float deltaTime) {

	}
};

class OwnerScript : public ScriptController<CompDataOwner, CompController<CompDataOwner>> {
public:
	OwnerScript(CompController< CompDataOwner>& cmpCtrl_, Engine& engine_) : ScriptController<CompDataOwner, CompController< CompDataOwner>>(cmpCtrl_, engine_) {}

	inline void executeSample(uint32_t id, CompDataOwner& data, World& world, float deltaTime) override {
		
	}
	inline void executeMeta(World& world, float deltaTime) {

	}
};

class SlaveScript : public ScriptController<CompDataSlave, CompController<CompDataSlave>> {
public:
	SlaveScript(CompController< CompDataSlave>& cmpCtrl_, Engine& engine_) : ScriptController<CompDataSlave, CompController< CompDataSlave>>(cmpCtrl_, engine_) {}

	inline void executeSample(uint32_t id, CompDataSlave& data, World& world, float deltaTime) override {

	}
	inline void executeMeta(World& world, float deltaTime) {

	}
};