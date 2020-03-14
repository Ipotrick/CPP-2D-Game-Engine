#pragma once

#include "Script.h"
#include "Engine.h"


class PlayerScript : public ScriptController<CompDataPlayer, CompController< CompDataPlayer>> {
public:
	PlayerScript(CompController< CompDataPlayer>& cmpCtrl_, Engine& engine_) : ScriptController<CompDataPlayer, CompController< CompDataPlayer>>(cmpCtrl_, engine_) {}

	inline void executeSample(uint32_t id, CompDataPlayer& data, World& world, float deltaTime) override {
		auto* entity = world.getEntityPtr(id);

		if (engine.keyPressed(KEY::W)) {
			entity->acceleration += rotate(vec2(0.0f, 10.0f), entity->rotation);

			//entity->position += vec2(0, 1) * dTime;
		}
		if (engine.keyPressed(KEY::A)) {
			entity->acceleration += rotate(vec2(-10.0f, 0.0f), entity->rotation);
			//entity->position += vec2(-1, 0) * dTime;
		}
		if (engine.keyPressed(KEY::S)) {
			entity->acceleration += rotate(vec2(0.0f, -10.0f), entity->rotation);
			//entity->position += vec2(0, -1) * dTime;
		}
		if (engine.keyPressed(KEY::D)) {
			entity->acceleration += rotate(vec2(10.0f, 0.0f), entity->rotation);
			//entity->position += vec2(1, 0) * dTime;
		}
		if (engine.keyPressed(KEY::Q)) {
			entity->rotation += 200.0f * deltaTime;
		}
		if (engine.keyPressed(KEY::E)) {
			entity->rotation -= 200.0f * deltaTime;
		}
		if (engine.keyPressed(KEY::F)) {
			vec2 bulletSize = vec2(0.08f, 0.08f);
			float bulletVel = 20.0f;
			float velOffsetRota = rand() % 20000 / 1000.0f - 10.0f;
			uint64_t bullets = data.bulletShotLapTimer.getLaps(deltaTime);
			for (uint64_t i = 0; i < bullets; i++) {
				Entity bullC = Entity(entity->getPos() + rotate(vec2(-entity->getHitboxSize().y, 0) / 2.0f, entity->rotation + 270), 0, Collidable(bulletSize, Form::CIRCLE, true, true, 1, 0.001f, entity->velocity + bulletVel * rotate(vec2(0, 1), entity->rotation + velOffsetRota)));
				CompDataDrawable bullD = CompDataDrawable(vec4(1, 0.f, 1.f, 0.3f), bulletSize, 0.2f, Form::CIRCLE);
				world.spawnEntity(bullC, bullD);
				world.bulletCompCtrl.registerEntity(world.getLastID(), CompDataBullet(10));
			}
		}
	}
	inline void executeMeta(World& world, float deltaTime) {

	}
};