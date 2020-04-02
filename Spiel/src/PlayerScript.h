#pragma once

#include "Script.h"
#include "Engine.h"


class PlayerScript : public ScriptController<Player> {
public:
	PlayerScript(Engine& engine_) : ScriptController<Player>(engine_) {}

	inline void executeSample(uint32_t entity, Player& data, float deltaTime) override {
		World& world = engine.world;
		auto [begin, end] = engine.getCollisionInfos(entity);
		for( auto iter = begin; iter != end; ++iter) {
			if (engine.world.hasComp<SolidBody>(iter->idB)) {
				engine.events.triggerEvent("playerHit");
			}
		}

		std::cout << "id: " << entity << std::endl;

		auto& baseEnt = world.getComp<Base>(entity);
		auto& movEnt = world.getComp<Movement>(entity);
		auto& collEnt = world.getComp<Collider>(entity);

		if (engine.keyPressed(KEY::W)) {
			//entity->acceleration += rotate(vec2(0.0f, 10.0f), entity->rotation);
			movEnt.velocity += rotate(vec2(0.0f, 10.0f), baseEnt.rotation) * deltaTime;
		}
		if (engine.keyPressed(KEY::A)) {
			//entity->acceleration += rotate(vec2(-10.0f, 0.0f), entity->rotation);
			movEnt.velocity += rotate(vec2(-10.0f, 0.0f), baseEnt.rotation) * deltaTime;
		}
		if (engine.keyPressed(KEY::S)) {
			//entity->acceleration += rotate(vec2(0.0f, -10.0f), entity->rotation);
			movEnt.velocity += rotate(vec2(0.0f, -10.0f), baseEnt.rotation) * deltaTime;
		}
		if (engine.keyPressed(KEY::D)) {
			//entity->acceleration += rotate(vec2(10.0f, 0.0f), entity->rotation);
			movEnt.velocity += rotate(vec2(10.0f, 0.0f), baseEnt.rotation) * deltaTime;
		}
		if (engine.keyPressed(KEY::Q)) {
			movEnt.angleVelocity += 700.0f * deltaTime;
		} 
		else if (engine.keyPressed(KEY::E)) {
			movEnt.angleVelocity -= 700.0f * deltaTime;
		}
		else {

		}
		
		if (engine.keyPressed(KEY::F)) {
			auto baseEnt = world.getComp<Base>(entity);
			auto movEnt = world.getComp<Movement>(entity);
			auto collEnt = world.getComp<Collider>(entity);

			float scale = rand() % 10 * 0.1f + 0.5f;
			vec2 bulletSize = vec2(0.05f, 0.05f) * scale;
			float bulletVel = 10.0f;
			uint64_t bullets = data.bulletShotLapTimer.getLaps(deltaTime);
			for (uint64_t i = 0; i < bullets; i++) {
				float velOffsetRota = rand() % 20000 / 1000.0f - 10.0f;
				vec2 bullCollVel = movEnt.velocity +bulletVel * rotate(vec2(0, 1), baseEnt.rotation + velOffsetRota);
				Collider bulletCollider = Collider(bulletSize, Form::CIRCLE, true, true);
				Draw bulletDraw = Draw(vec4(0.f, 1.f, 0.f, 1), bulletSize, 0.4f, Form::CIRCLE);
				auto bullet = world.createEnt();
				world.addComp<Base>(bullet, Base(baseEnt.position + rotate(vec2(-collEnt.size.y, 0) / 1.5f, baseEnt.rotation + 270)));
				world.addComp<Movement>(bullet, Movement(bullCollVel, 0));
				world.addComp<SolidBody>(bullet, SolidBody(0.9f, 0.01f, 1));
				world.addComp<Draw>(bullet, bulletDraw);
				world.addComp<Collider>(bullet, bulletCollider);
				world.addComp<Bullet>(world.getLastEntID(), Bullet(10 * scale));
			}
		}
	}
	inline void executeMeta(float deltaTime) override {

	}
};