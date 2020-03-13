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


class TriggerScript : public ScriptController<CompDataTrigger, CompController<CompDataTrigger>> {
public:
	TriggerScript(CompController< CompDataTrigger>& cmpCtrl_, Engine& engine_) : ScriptController<CompDataTrigger, CompController< CompDataTrigger>>(cmpCtrl_, engine_) {}

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
						Entity midComp1C = Entity(vec2(0, -4), 0, Collidable(vec2(3, 0.4f), Form::RECTANGLE, false, true));
						CompDataDrawable midComp1D = CompDataDrawable(vec4(1, 0, 0, 1), vec2(4, 2.5f), 0.49f, Form::RECTANGLE);
						world.spawnEntity(midComp1C, midComp1D);

						Entity midComp2C = Entity(vec2(0, -1.5f), 0, Collidable(vec2(3, 0.4f), Form::RECTANGLE, false, true));
						CompDataDrawable midComp2D = CompDataDrawable(vec4(1, 0, 0, 1), vec2(2, 1.5), 0.49f, Form::RECTANGLE);
						world.spawnEntity(midComp2C, midComp2D);

						Entity midComp3C = Entity(vec2(0, 0), 0, Collidable(vec2(3, 0.4f), Form::RECTANGLE, false, true));
						CompDataDrawable midComp3D = CompDataDrawable(vec4(1, 0, 0, 1), vec2(0.75f, 1), 0.49f, Form::RECTANGLE);
						world.spawnEntity(midComp3C, midComp3D);
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