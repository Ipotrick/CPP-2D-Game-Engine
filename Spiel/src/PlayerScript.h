#pragma once

#include "Script.h"
#include "Engine.h"


class PlayerScript : public ScriptController<CompDataPlayer, CompController< CompDataPlayer>> {
public:
	PlayerScript(CompController< CompDataPlayer>& cmpCtrl_, Engine& engine_) : ScriptController<CompDataPlayer, CompController< CompDataPlayer>>(cmpCtrl_, engine_) {}

	inline void executeSample(uint32_t id, CompDataPlayer& data, World& world, float deltaTime) override {
		auto [begin, end] = engine.getCollisionInfos(id);
		for( auto iter = begin; iter != end; ++iter) {
			if (engine.world.getEntity(iter->idB).isSolid()) {
				engine.events.triggerEvent("playerHit");
			}
		}

		auto& entity = world.getEntity(id);

		if (engine.keyPressed(KEY::W)) {
			//entity->acceleration += rotate(vec2(0.0f, 10.0f), entity->rotation);
			entity.velocity += rotate(vec2(0.0f, 10.0f), entity.rotation) * deltaTime;
		}
		if (engine.keyPressed(KEY::A)) {
			//entity->acceleration += rotate(vec2(-10.0f, 0.0f), entity->rotation);
			entity.velocity += rotate(vec2(-10.0f, 0.0f), entity.rotation) * deltaTime;
		}
		if (engine.keyPressed(KEY::S)) {
			//entity->acceleration += rotate(vec2(0.0f, -10.0f), entity->rotation);
			entity.velocity += rotate(vec2(0.0f, -10.0f), entity.rotation) * deltaTime;
		}
		if (engine.keyPressed(KEY::D)) {
			//entity->acceleration += rotate(vec2(10.0f, 0.0f), entity->rotation);
			entity.velocity += rotate(vec2(10.0f, 0.0f), entity.rotation) * deltaTime;
		}
		if (engine.keyPressed(KEY::Q)) {
			entity.angleVelocity += 700.0f * deltaTime;
		} 
		else if (engine.keyPressed(KEY::E)) {
			entity.angleVelocity -= 700.0f * deltaTime;
		}
		else {

		}
		if (engine.keyPressed(KEY::F)) {
			float scale = rand() % 10 * 0.1f + 0.5f;
			vec2 bulletSize = vec2(0.05f, 0.05f) * scale;
			float bulletVel = 10.0f;
			float velOffsetRota = rand() % 20000 / 1000.0f - 10.0f;
			uint64_t bullets = data.bulletShotLapTimer.getLaps(deltaTime);
			for (uint64_t i = 0; i < bullets; i++) {
				
				vec2 bullCollVel = world.getEntity(id).getPos() + bulletVel * rotate(vec2(0, 1), world.getEntity(id).getRota() + velOffsetRota);

				Collidable bullColl = Collidable(bulletSize, Form::CIRCLE, true, bullCollVel);
				Entity bullC = Entity(world.getEntity(id).getPos() + rotate(vec2(-world.getEntity(id).getSize().y, 0) / 1.9f, world.getEntity(id).rotation + 270), 0, bullColl);
				bullC.particle = true;
				CompDataDrawable bullD = CompDataDrawable(vec4(0.f, 1.f, 0.f, 1), bulletSize, 0.4f, Form::CIRCLE);
				world.spawnSolidEntity(bullC ,bullD, CompDataSolidBody(1, 0.00001f));
				world.bulletCompCtrl.registerEntity(world.getLastID(), CompDataBullet(10 * scale));
			}
		}
	}
	inline void executeMeta(World& world, float deltaTime) {

	}
};