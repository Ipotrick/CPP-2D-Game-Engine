#include "PlayerScript.hpp"

void playerScript(EntityHandle me, Player& data, float deltaTime)
{
	World& world = Engine::world;
	auto cmps = Engine::world.componentView(me);

	for (auto ent : world.entityView<Player>()) {
		Engine::renderer.getCamera().position = world.getComp<Transform>(ent).position;
	}

	auto spawnParticles = [&](int num, Vec2 dir, float vel, Vec2 offset) {
		for (int i = 0; i < num; i++) {
			auto particle = world.create();
			Transform base = world.getComp<Transform>(me);
			float rando = (rand() % 1000 / 800.0f);
			base.position += offset + offset * rando;
			world.addComp<Transform>(particle, base);
			Movement mov = world.getComp<Movement>(me);
			mov.angleVelocity += (rand() % 1000 / 400.0f * 90.0f - 45.0f)*4;
			mov.velocity.x += (rand() % 1000 / 400.0f - 1.25f)*4;
			mov.velocity.y += (rand() % 1000 / 400.0f - 1.25f)*4;
			mov.velocity += dir * vel * 1.8;
			world.addComp<Draw>(particle, Draw(Vec4(0,0,0,0), Vec2(0,0), rand() % 1000 * 0.00052f + 0.04f, Form::Circle));
			if (rand() % 4 == 0) {
				world.addComp<Age>(particle, Age(3.5f));
				world.addComp<ParticleScriptComp>(particle, ParticleScriptComp(Vec2(0.1f, 0.1f), Vec2(10, 10),
					Vec4(200 / 256.f, 20 / 256.f, 1 / 256.f, 0.95),
					Vec4(0.1, 0.1, 0.1, 0)
				));
			}
			else {
				world.addComp<Age>(particle, Age(0.5f));
				world.addComp<ParticleScriptComp>(particle, ParticleScriptComp(Vec2(0.1f, 0.1f), Vec2(2, 2),
					Vec4(200 / 256.f, 20 / 256.f, 1 / 256.f, 0.95),
					Vec4(120 / 256.f, 10 / 256.f, 1 / 256.f, 0.0f)
				));
				mov.velocity * 2;
			}
			world.addComp<Movement>(particle, mov);
			world.addComp<PhysicsBody>(particle, PhysicsBody(0.9f, 0.00000002f, 0.0001, 0));
			auto coll = Collider(Vec2(0.2f, 0.2f), Form::Circle, true);
			coll.ignoreGroupMask |= CollisionGroup<1>::mask;
			world.addComp<Collider>(particle, coll);
			world.addComp(particle, Engine::renderer.makeTexRef(TextureInfo("Cloud.png")));
			world.spawn(particle);
			world.getComp<Age>(particle).curAge += rando * 0.02f;
			cmps.get<Movement>().velocity -= mov.velocity * world.getComp<PhysicsBody>(particle).mass*100000 / world.getComp<PhysicsBody>(me).mass*10;
		}
	};

	float powerAdjust = 5.0f;
	float minPower = 0.05f;
	float maxPower = 10.0f;

	if (Engine::in.keyPressed(Key::LEFT_SHIFT)) {
		data.power = std::min(data.power + deltaTime * powerAdjust, maxPower);
		printf("new player power: %f\n", data.power);
	}
	if (Engine::in.keyPressed(Key::LEFT_CONTROL)) {
		data.power = std::max(data.power - deltaTime * powerAdjust, minPower);
		printf("new player power: %f\n", data.power);
	}
	data.flameSpawnTimer.setLapTime(0.008 * (1 / data.power));

	if (Engine::in.keyPressed(Key::Q)) {
		world.getComp<Movement>(me).angleVelocity += 100 * deltaTime;
	}
	if (Engine::in.keyPressed(Key::E)) {
		world.getComp<Movement>(me).angleVelocity -= 100 * deltaTime;
	}
	world.getComp<Movement>(me).angleVelocity *= 1 - deltaTime * 10;

	if (Engine::in.keyPressed(Key::SPACE)) {
		auto num = data.flameSpawnTimer.getLaps(deltaTime);
		spawnParticles(num, rotate(Vec2(-1, 0), cmps.get<Transform>().rotation + 90), 20, rotate(Vec2(-1, 0), cmps.get<Transform>().rotation + 90) * (0.4f) );

	}

	if (Engine::in.keyPressed(Key::F)) {
		auto const& baseEnt = cmps.get<Transform>();
		auto movEnt = cmps.get<Move>();
		auto collEnt = cmps.get<Coll>();

		Vec2 scale(0.4, 0.4);
		float bulletVel = 20.0f;
		uint64_t bullets = data.bulletShotLapTimer.getLaps(deltaTime) * data.power;
		for (uint64_t i = 0; i < bullets; i++) {
			float velOffsetRota = rand() % 20000 / 1000.0f - 10.0f;
			Vec2 bullCollVel = movEnt.velocity + (bulletVel + (rand() % 1000 / 1000.0f)) * rotate(Vec2(0, 1), baseEnt.rotation + velOffsetRota);
			Collider bulletCollider = Collider(scale, Form::Circle, true);
			Draw bulletDraw = Draw(Vec4(0.f, 1.f, 0.f, 1), scale, 0.4f, Form::Circle);
			auto bullet = world.create();
			world.addComp<Transform>(bullet, Transform(baseEnt.position + rotate(Vec2(-collEnt.size.y, 0.0f) * 1.3f, baseEnt.rotation + 270)));
			world.addComp<Movement>(bullet, Movement(bullCollVel, 0));
			world.addComp<PhysicsBody>(bullet, PhysicsBody(0.9f, 0.01f, 1, 0));
			world.addComp<Draw>(bullet, bulletDraw);
			world.addComp<Collider>(bullet, bulletCollider);
			world.addComp<Bullet>(bullet, Bullet(1, 3));
			world.spawn(bullet);
		}
	}
}