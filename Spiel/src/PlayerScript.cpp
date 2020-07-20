#pragma once

#include "PlayerScript.h"

void PlayerScript::script(entity_id me, Player& data, float deltaTime) {
	World& world = engine.world;
	auto cmps = world.viewComps(me);
	auto [begin, end] = engine.getCollisions(me);
	for (auto iter = begin; iter != end; ++iter) {
		if (engine.world.hasComp<PhysicsBody>(iter->idB)) {
			engine.events.triggerEvent("playerHit");
		}
	}

	auto spawnParticles = [&](int num, Vec2 dir, float vel, Vec2 offset) {
		for (int i = 0; i < num; i++) {
			auto particle = world.create();
			Base base = world.getComp<Base>(me);
			base.position += offset;
			world.addComp<Base>(particle, base);
			Movement mov = world.getComp<Movement>(me);
			mov.angleVelocity += rand() % 1000 / 400.0f * 90.0f - 45.0f;
			mov.velocity.x += rand() % 1000 / 400.0f * 1.0f - 0.5f;
			mov.velocity.y += rand() % 1000 / 400.0f * 1.0f - 0.5f;
			mov.velocity += dir * vel;
			world.addComp<Movement>(particle, mov);
			world.addComp<Draw>(particle, Draw(Vec4(0,0,0,0), Vec2(0,0), 0.49f, Form::Circle));
			if (rand() % 4 == 0) {
				world.addComp<Age>(particle, Age(1.5f));
				world.addComp<ParticleScriptComp>(particle, ParticleScriptComp(Vec2(0.01f, 0.01f), Vec2(10, 10),
					Vec4(220 / 256.f, 20 / 256.f, 20 / 256.f, 0.9f),
					Vec4(0, 0, 0, 0)
				));
			}
			else {
				world.addComp<Age>(particle, Age(1.0f));
				world.addComp<ParticleScriptComp>(particle, ParticleScriptComp(Vec2(0.8f, 0.8f), Vec2(4, 4),
					Vec4(202 / 256.f, 40 / 256.f, 20 / 256.f, 0.9f),
					Vec4(0, 0, 0, 0)
				));
			}
			world.addComp<PhysicsBody>(particle, PhysicsBody(0.9f, 0.001f, 0.0001, 0));
			world.addComp<Collider>(particle, Collider(Vec2(0.2f, 0.2f), Form::Circle, true));
			world.addComp<TextureRef>(particle, TextureRef("Cloud.png"));
			world.spawnLater(particle);
		}
	};

	auto cursorPos = engine.getPosWorldSpace(engine.getCursorPos());

	auto player_to_cursor = normalize(cursorPos - cmps.get<Base>().position);
	auto newRotation = getRotation(player_to_cursor) - 90;
	cmps.get<Movement>().angleVelocity = (newRotation - cmps.get<Base>().rotation);
	cmps.get<Base>().rotation = newRotation;

	float maxVel = 10.0f;
	float const maxAccel = 20.0f;	// (1 unit/second^2)

	if (engine.keyPressed(KEY::SPACE)) maxVel *= 2.5;
	
	// calculate accelerationDir:
	Vec2 accellDir(0, 0);
	bool accell{ false };
	if (engine.keyPressed(KEY::W)) {
		accellDir += Vec2(0, 1);
		accell = true;
	}
	if (engine.keyPressed(KEY::A)) {
		accellDir += Vec2(-1, 0);
		accell = true;
	}
	if (engine.keyPressed(KEY::S)) {
		accellDir += Vec2(0, -1);
		accell = true;
	}
	if (engine.keyPressed(KEY::D)) {
		accellDir += Vec2(1, 0);
		accell = true;
	}
	if (accell) {
		Vec2 newVel = cmps.get<Movement>().velocity + accellDir * maxAccel * deltaTime;
		Vec2 newVelDir = normalize(newVel);
		float newVelAbs = std::min(maxVel, length(newVel));
		cmps.get<Movement>().velocity = newVelDir * newVelAbs;
	}

	if (engine.keyPressed(KEY::C)) {
		auto num = data.flameSpawnTimer.getLaps(deltaTime);
		spawnParticles(num, rotate(Vec2(1, 0), cmps.get<Base>().rotation + 90), 15, rotate(Vec2(1, 0), cmps.get<Base>().rotation + 90) * 0.6f);

	}

	if (engine.keyPressed(KEY::F)) {
		auto baseEnt = cmps.get<Base>();
		auto movEnt = cmps.get<Move>();
		auto collEnt = cmps.get<Coll>();

		Vec2 scale(0.4, 0.4);
		float bulletVel = 20.0f;
		uint64_t bullets = data.bulletShotLapTimer.getLaps(deltaTime);
		for (uint64_t i = 0; i < bullets; i++) {
			float velOffsetRota = rand() % 20000 / 1000.0f - 10.0f;
			Vec2 bullCollVel = movEnt.velocity + (bulletVel + (rand() % 1000 / 1000.0f)) * rotate(Vec2(0, 1), baseEnt.rotation + velOffsetRota);
			Collider bulletCollider = Collider(scale, Form::Circle, true);
			Draw bulletDraw = Draw(Vec4(0.f, 1.f, 0.f, 1), scale, 0.4f, Form::Circle);
			auto bullet = world.create();
			world.addComp<Base>(bullet, Base(baseEnt.position + rotate(Vec2(-collEnt.size.y, 0.0f) * 1.3f, baseEnt.rotation + 270)));
			world.addComp<Movement>(bullet, Movement(bullCollVel, 0));
			world.addComp<PhysicsBody>(bullet, PhysicsBody(0.9f, 0.01f, 1, 0));
			world.addComp<Draw>(bullet, bulletDraw);
			world.addComp<Collider>(bullet, bulletCollider);
			world.addComp<Bullet>(bullet, Bullet(10, 3));
			world.spawn(bullet);
		}
	}

	
	if (engine.keyPressed(KEY::K) && !world.isIdValid(data.dummyExis)) {
		std::cout << "stored ID " << data.dummyExis.id << std::endl;
		auto baseEnt = cmps.get<Base>();
		auto movEnt = cmps.get<Move>();
		auto collEnt = cmps.get<Coll>();

		Vec2 scale(0.8, 0.8);
		float dummyVel = 0.0f;

		float velOffsetRota = rand() % 20000 / 1000.0f - 10.0f;
		Collider DummyCollider = Collider(scale, Form::Circle, false);
		Draw dummyDraw = Draw(Vec4(1.f, 1.f, 1.f, 1), scale, 0.4f, Form::Circle);
		
		auto dummy = world.create();
		world.addComp<Base>(dummy, Base(baseEnt));
		world.addComp<Movement>(dummy);
		world.addComp<PhysicsBody>(dummy, PhysicsBody(0.9f, 0.01f, 1, 0));
		world.addComp<Draw>(dummy, dummyDraw);
		world.addComp<Collider>(dummy, DummyCollider);
		world.addComp<Dummy>(dummy, Dummy(me));
		world.addComp<Health>(dummy, 100);
		world.spawn(dummy);
		if (!world.isIdValid(data.dummyExis))
		{
			std::cout << " error, id is not valid " << std::endl;
		}
	}
}