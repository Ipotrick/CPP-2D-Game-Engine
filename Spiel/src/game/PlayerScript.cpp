#include "PlayerScript.hpp"

void playerScript(EntityHandle me, Player& data, float deltaTime)
{
	World& world = Game::world;
	auto cmps = Game::world.componentView(me);

	for (auto ent : world.entityView<Player>()) {
		EngineCore::renderer.getCamera().position = world.getComp<Transform>(ent).position;
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
			world.addComp<Draw>(particle, Draw(Vec4(0,0,0,0), Vec2(0,0), 0.6f, Form::Circle, true));
			if (rand() % 4 == 0) {
				world.addComp<Age>(particle, Age(1.2f));
				world.addComp<ParticleScriptComp>(particle, ParticleScriptComp(Vec2(0.1f, 0.1f), Vec2(10, 10),
					Vec4(255 / 256.f, 30 / 256.f, 20 / 256.f, 0.7),
					Vec4(255 / 256.f, 30 / 256.f, 20 / 256.f, 0)
				));
			}
			else {
				world.addComp<Age>(particle, Age(0.5f));
				world.addComp<ParticleScriptComp>(particle, ParticleScriptComp(Vec2(0.1f, 0.1f), Vec2(2, 2),
					Vec4(255 / 256.f, 30 / 256.f, 20 / 256.f, 0.7),
					Vec4(255 / 256.f, 30 / 256.f, 20 / 256.f, 0)
				));
				mov.velocity * 2;
			}
			world.addComp<Movement>(particle, mov);
			world.addComp<PhysicsBody>(particle, PhysicsBody(0.9f, 0.00000002f, 0.0001, 0));
			auto coll = Collider(Vec2(0.2f, 0.2f), Form::Circle, true);
			coll.ignoreGroupMask |= CollisionGroup<1>::mask;
			world.addComp<Collider>(particle, coll);
			world.addComp(particle, EngineCore::renderer.makeTexRef(TextureDiscriptor("Cloud.png")));
			world.spawn(particle);
			world.getComp<Age>(particle).curAge += rando * 0.02f;
			cmps.get<Movement>().velocity -= mov.velocity * world.getComp<PhysicsBody>(particle).mass*100000 / world.getComp<PhysicsBody>(me).mass*10;
		}
	};

	float powerAdjust = 5.0f;
	float minPower = 0.05f;
	float maxPower = 10.0f;

	if (EngineCore::in.keyPressed(Key::LEFT_SHIFT)) {
		data.power = std::min(data.power + deltaTime * powerAdjust, maxPower);
	}
	if (EngineCore::in.keyPressed(Key::LEFT_CONTROL)) {
		data.power = std::max(data.power - deltaTime * powerAdjust, minPower);
	}
	data.flameSpawnTimer.setLapTime(0.008 * (1 / data.power));

	if (EngineCore::in.keyPressed(Key::Q)) {
		world.getComp<Movement>(me).angleVelocity += 100 * deltaTime;
	}
	if (EngineCore::in.keyPressed(Key::E)) {
		world.getComp<Movement>(me).angleVelocity -= 100 * deltaTime;
	}
	world.getComp<Movement>(me).angleVelocity *= 1 - deltaTime * 10;

	if (EngineCore::in.keyPressed(Key::SPACE)) {
		auto num = data.flameSpawnTimer.getLaps(deltaTime);
		spawnParticles(num, rotate(-cmps.get<Transform>().rotaVec.toUnitX0(), 90), 20, rotate(-cmps.get<Transform>().rotaVec.toUnitX0(), 90) * (0.4f) );

	}

	auto const& playerTransform = cmps.get<Transform>();
	auto playerMovement = cmps.get<Move>();
	auto playerCollider = cmps.get<Coll>();

	auto spawnBullet = [&](Vec2 vel, Vec2 offset, float size) {
		Vec2 scale = Vec2{ 1,1 } * size;
		Collider bulletCollider = Collider(scale, Form::Circle, true);
		Draw bulletDraw = Draw(Vec4(0.3f, 1.5f, 0.3f, 1), scale, 0.4f, Form::Circle);
		auto bullet = world.create();
		world.addComp<Transform>(bullet, Transform(playerTransform.position + offset));
		world.addComp<Movement>(bullet, Movement(vel, 0));
		world.addComp<PhysicsBody>(bullet, PhysicsBody(0.9f, 0.01f, 1, 0));
		world.addComp<Draw>(bullet, bulletDraw);
		world.addComp<Collider>(bullet, bulletCollider);
		world.addComp<Bullet>(bullet, Bullet(1, 3));
		world.addComp<Age>(bullet, Age(2));
		world.spawn(bullet);
	};

	if (EngineCore::in.keyJustPressed(Key::C)) {
		constexpr int BULLET_COUNT{ 300 };
		constexpr float BULLET_VEL = 10.0f;
		for (int i = 0; i < BULLET_COUNT; ++i) {
			float angle = float(i) / float(BULLET_COUNT) * 360;
			Vec2 bullCollVel = playerMovement.velocity + BULLET_VEL * rotate(Vec2(0, 1), angle);
			Vec2 offset = Vec2(1, 1) * rotate(Vec2(0, 1), angle);
			const float size = 1;
			spawnBullet(bullCollVel, offset, size);
		}
	}

	if (EngineCore::in.keyPressed(Key::F)) {
		constexpr float BULLET_VEL = 40.0f;
		constexpr float SIZE = 0.4f;
		constexpr float SPREAD_ANGLE = 40.0f;

		std::random_device rd;
		std::mt19937 rand_generator(rd());
		std::uniform_real_distribution rotation_distr(-SPREAD_ANGLE / 2.0f, SPREAD_ANGLE / 2.0f);

		uint64_t bullets = data.bulletShotLapTimer.getLaps(deltaTime);
		for (uint64_t i = 0; i < bullets; i++) {
			Vec2 bullDir = rotate(playerTransform.rotaVec.toUnitY0(), rotation_distr(rand_generator));
			Vec2 bulletVelocity = playerMovement.velocity + bullDir * BULLET_VEL;
			Vec2 offset = bullDir;
			spawnBullet(bulletVelocity, offset, SIZE);
		}
	}
}