#pragma once

#include "Script.h"
#include "Engine.h"


class HealthScript : public ScriptController<Health, CompController< Health>> {
public:
	HealthScript(CompController<Health>& cmpCtrl_, Engine& engine_) : ScriptController<Health, CompController< Health>>(cmpCtrl_, engine_) {}

	inline void executeSample(uint32_t id, Health& data, World& world, float deltaTime) override {
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
	inline void executeMeta(World& world, float deltaTime) {

	}
};



class AgeScript : public ScriptController<Age, CompController< Age>> {
public:
	AgeScript(CompController< Age>& cmpCtrl_, Engine& engine_) : ScriptController<Age, CompController< Age>>(cmpCtrl_, engine_) {}

	inline void executeSample(uint32_t id, Age& data, World& world, float deltaTime) override {

		data.curAge += deltaTime;

		if (data.curAge > data.maxAge) {
			world.despawn(id);
		}
	}
	inline void executeMeta(World& world, float deltaTime) {

	}
};



class BulletScript : public ScriptController<Bullet, CompController<Bullet>> {
public:
	BulletScript(CompController< Bullet>& cmpCtrl_, Engine& engine_) : ScriptController<Bullet, CompController< Bullet>>(cmpCtrl_, engine_) {}

	inline void executeSample(uint32_t id, Bullet& data, World& world, float deltaTime) override {
		auto [begin, end] = engine.getCollisionInfos(id);
		bool foundCollisionWithMortal{ false };
		for (auto iter = begin; iter != end; ++iter) {
			if (world.hasComp<Health>(iter->idB)) {
				world.getCompPtr<Health>(iter->idB)->curHealth -= data.damage;
				foundCollisionWithMortal = true;
			}
		}

		if (foundCollisionWithMortal) {
			world.despawn(id);
			auto newEnt = world.getEntityPtr(id);
			newEnt->size *= 0.5f;
			if (norm(newEnt->size) > 0.02f) {
				newEnt->velocity *= 0.5f;
				auto newDrawable = world.getCompPtr<Draw>(id);
				newDrawable->scale *= 0.5f;
				world.spawnSolidEntity(*newEnt, *newDrawable, world.getComp<SolidBody>(id));
				world.addComp<Bullet>(world.getLastID(), Bullet(data.damage * 0.5f));
			}
		}
	}
	inline void executeMeta(World& world, float deltaTime) {

	}
};


class TriggerScript : public ScriptController<Trigger, CompController<Trigger>> {
public:
	TriggerScript(CompController< Trigger>& cmpCtrl_, Engine& engine_) : ScriptController<Trigger, CompController< Trigger>>(cmpCtrl_, engine_), spawnTimer{ 0.001 } {}
	LapTimer<> spawnTimer;


	inline void executeSample(uint32_t id, Trigger& data, World& world, float deltaTime) override {
		auto begin = world.getAllComps<Player>().begin();
		auto end = world.getAllComps<Player>().end();
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
						Draw trashEntD = Draw(vec4(1, 1, 1, 1), scale, 0.5f, Form::CIRCLE, true);

						for (int i = 0; i < spawnTimer.getLaps(deltaTime); i++) {
							world.spawnSolidEntity(trashEntC,trashEntD, SolidBody(0.9f, 0.5f));
							world.addComp<Health>(world.getLastID(), Health(100));
						}
					}
				}
			}
		}
	}
	inline void executeMeta(World& world, float deltaTime) {

	}
};