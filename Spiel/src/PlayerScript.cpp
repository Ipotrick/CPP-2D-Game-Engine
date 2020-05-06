#pragma once

#include "PlayerScript.h"

void PlayerScript::script(entity_handle entity, Player& data, float deltaTime) {
	World& world = engine.world;
	auto cmps = world.viewComps(entity);
	auto [begin, end] = engine.getCollisions(entity);
	for (auto iter = begin; iter != end; ++iter) {
		if (engine.world.hasComp<PhysicsBody>(iter->idB)) {
			engine.events.triggerEvent("playerHit");
		}
	}

	auto spawnParticles = [&](int num, Vec2 dir, float vel, Vec2 offset) {
		for (int i = 0; i < num; i++) {
			auto particle = world.createEnt();
			Base base = world.getComp<Base>(entity);
			base.position += offset;
			world.addComp<Base>(particle, base);
			Movement mov = world.getComp<Movement>(entity);
			mov.angleVelocity += rand() % 1000 / 400.0f * 90.0f - 45.0f;
			mov.velocity.x += rand() % 1000 / 400.0f * 1.0f - 0.5f;
			mov.velocity.y += rand() % 1000 / 400.0f * 1.0f - 0.5f;
			mov.velocity += dir * vel;
			world.addComp<Movement>(particle, mov);
			world.addComp<Draw>(particle, Draw(Vec4(0,0,0,0), Vec2(0,0), 0.49f, Form::CIRCLE));
			if (rand() % 4 == 0) {
				world.addComp<Age>(particle, Age(1.5f));
				world.addComp<ParticleScriptComp>(particle, ParticleScriptComp(Vec2(0.01f, 0.01f), Vec2(3, 3),
					Vec4(220 / 256.f, 20 / 256.f, 20 / 256.f, 0.9f),
					Vec4(0, 0, 0, 0)
				));
			}
			else {
				world.addComp<Age>(particle, Age(1.0f));
				world.addComp<ParticleScriptComp>(particle, ParticleScriptComp(Vec2(0.2f, 0.2f), Vec2(1, 1),
					Vec4(202 / 256.f, 40 / 256.f, 20 / 256.f, 0.9f),
					Vec4(0, 0, 0, 0)
				));
			}
			world.addComp<PhysicsBody>(particle, PhysicsBody(0.9f, 0.01f, 0.0001, 0));
			world.addComp<Collider>(particle, Collider(Vec2(0.2f, 0.2f), Form::CIRCLE, true));
			world.addComp<TextureRef>(particle, TextureRef("Cloud.png"));
			world.spawnLater(particle);
		}
	};

	if (engine.keyPressed(KEY::LEFT_SHIFT)) {
		data.power += 1 * deltaTime;
		data.power = std::min(data.power, 10.0f);
		std::cout << "power: " << data.power << std::endl;
	}

	if (engine.keyPressed(KEY::LEFT_CONTROL)) {
		data.power -= 1 * deltaTime;
		data.power = std::max(data.power, 0.1f);
		std::cout << "power: " << data.power << std::endl;
	}

	float lapTime = 1.f / (data.power * 1'000.0f);
	data.flameSpawnTimer.setLapTime(lapTime);
	float flames = data.flameSpawnTimer.getLaps(deltaTime);

	if (engine.keyPressed(KEY::W)) {
		cmps.get<Move>().velocity += rotate(Vec2(0.0f, 2.0f*data.power), cmps.get<Base>().rotation) * deltaTime;
		spawnParticles(flames, rotate(rotate(Vec2(1, 0), cmps.get<Base>().rotaVec), 270), 5, rotate(Vec2(0,-world.getComp<Collider>(entity).size.y * 0.7f), cmps.get<Base>().rotaVec));
	}
	if (engine.keyPressed(KEY::A)) {
		cmps.get<Move>().velocity += rotate(Vec2(-1.0f * data.power, 0.0f), cmps.get<Base>().rotation) * deltaTime;
		spawnParticles(flames, rotate(rotate(Vec2(1, 0), cmps.get<Base>().rotaVec), 0), 5, rotate(Vec2(world.getComp<Collider>(entity).size.x * 0.5f, 0), cmps.get<Base>().rotaVec));
	}
	if (engine.keyPressed(KEY::S)) {
		cmps.get<Move>().velocity += rotate(Vec2(0.0f, -1.0f * data.power), cmps.get<Base>().rotation) * deltaTime;
		spawnParticles(flames, rotate(rotate(Vec2(1, 0), cmps.get<Base>().rotaVec), 90), 5, rotate(Vec2(0, world.getComp<Collider>(entity).size.y * 0.7f), cmps.get<Base>().rotaVec));
	}
	if (engine.keyPressed(KEY::D)) {
		cmps.get<Move>().velocity += rotate(Vec2(1.0f * data.power, 0.0f), cmps.get<Base>().rotation) * deltaTime;
		spawnParticles(flames, rotate(rotate(Vec2(1, 0), cmps.get<Base>().rotaVec), 180), 5, rotate(Vec2(-world.getComp<Collider>(entity).size.x * 0.5f, 0), cmps.get<Base>().rotaVec));
	}


	cmps.get<Move>().angleVelocity *= 1 / (1 + 2*deltaTime);
	if (engine.keyPressed(KEY::Q)) {
		cmps.get<Move>().angleVelocity += 700.0f * deltaTime;
	}
	else if (engine.keyPressed(KEY::E)) {
		cmps.get<Move>().angleVelocity -= 700.0f * deltaTime;
	}

	if (engine.keyPressed(KEY::F)) {
		auto baseEnt = cmps.get<Base>();
		auto movEnt = cmps.get<Move>();
		auto collEnt = cmps.get<Coll>();

		float scale = rand() % 10 * 0.1f + 0.5f;
		Vec2 bulletSize = Vec2(0.05f, 0.05f) * scale;
		float bulletVel = 5.0f;
		uint64_t bullets = data.bulletShotLapTimer.getLaps(deltaTime);
		for (uint64_t i = 0; i < bullets; i++) {
			float velOffsetRota = rand() % 20000 / 1000.0f - 10.0f;
			Vec2 bullCollVel = movEnt.velocity + (bulletVel + (rand() % 1000 / 1000.0f)) * rotate(Vec2(0, 1), baseEnt.rotation + velOffsetRota);
			Collider bulletCollider = Collider(bulletSize, Form::CIRCLE, true);
			Draw bulletDraw = Draw(Vec4(0.f, 1.f, 0.f, 1), bulletSize, 0.4f, Form::CIRCLE);
			auto bullet = world.createEnt();
			world.addComp<Base>(bullet, Base(baseEnt.position + rotate(Vec2(-collEnt.size.y, 0.0f) / 1.5f, baseEnt.rotation + 270)));
			world.addComp<Movement>(bullet, Movement(bullCollVel, 0));
			world.addComp<PhysicsBody>(bullet, PhysicsBody(0.9f, 0.01f, 1, 0));
			world.addComp<Draw>(bullet, bulletDraw);
			world.addComp<Collider>(bullet, bulletCollider);
			world.addComp<Bullet>(world.getLastEntity(), Bullet(10.0f * scale));
			world.spawn(bullet);
		}
	}
}