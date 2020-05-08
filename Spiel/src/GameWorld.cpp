#include "GameWorld.h"
#include "Physics.h"

void World::loadMap(std::string mapname_) {
	std::ifstream mapData(mapname_);
	if (mapData.good()) {

	}
	else
	{
		Vec2 scaleEnt = { 0.4f, 0.8f };
		physics.friction = 0.16f;
		//uniformsPhysics.linearEffectDir = Vec2(0, -1);
		//uniformsPhysics.linearEffectAccel = 1.f;

		auto player = create();
		addComp<Base>(player, Base({ 0,0 }, 0));
		addComp<Movement>(player, Movement(0.0f, 0.0f));
		addComp<Draw>(player, Draw(Vec4(1, 1, 1, 1), scaleEnt, 0.6f, Form::RECTANGLE));
		addComp<Collider>(player, Collider(scaleEnt, Form::RECTANGLE));
		addComp<PhysicsBody>(player, PhysicsBody(0.1f, 60, calcMomentOfIntertia(60, scaleEnt), 100));
		addComp<TextureRef>(player, TextureRef("Dir.png"));
		addComp<Player>(player, Player());
		spawn(player);

		auto slave = create();
		addComp<Base>(slave);
		addComp<Movement>(slave);
		addComp<PhysicsBody>(slave);
		addComp<Collider>(slave, Collider({ scaleEnt.x * 1 / sqrtf(2.0f) }, Form::RECTANGLE));
		addComp<Draw>(slave, Draw(Vec4(0, 0, 0, 1), { scaleEnt.x * 1 / sqrtf(2.0f) }, 0.6f, Form::RECTANGLE));
		link(slave, player, Vec2(0.0f, 0.4f), 45.0f);
		spawn(slave);

		Vec2 scaleLegs{ 0.1, 0.2 };
		slave = create();
		addComp<Base>(slave);
		addComp<Movement>(slave);
		addComp<PhysicsBody>(slave);
		addComp<Collider>(slave, Collider(scaleLegs, Form::RECTANGLE));
		addComp<Draw>(slave, Draw(Vec4(0, 0, 0, 1), scaleLegs, 0.6f, Form::RECTANGLE));
		link(slave, player, Vec2(0.2f, -0.4f), 30.0f);
		spawn(slave);


		slave = create();
		addComp<Base>(slave);
		addComp<Movement>(slave);
		addComp<PhysicsBody>(slave);
		addComp<Collider>(slave, Collider(scaleLegs, Form::RECTANGLE));
		addComp<Draw>(slave, Draw(Vec4(0, 0, 0, 1), scaleLegs, 0.6f, Form::RECTANGLE));
		link(slave, player, Vec2(-0.2f, -0.4f), -30.0f);
		spawn(slave);

		/*Vec2 scaleEnemy{ 5.4f, 1.4f };
		auto enemy = createEnt();
		addComp<Base>(enemy, Base({ 0,0 }, 0));
		addComp<Movement>(enemy, Movement(0.0f, 0.0f));
		addComp<Draw>(enemy, Draw(Vec4(1, 1, 1, 1), scaleEnemy, 0.4f, Form::RECTANGLE));
		addComp<Collider>(enemy, Collider(scaleEnemy, Form::RECTANGLE));
		addComp<PhysicsBody>(enemy, PhysicsBody(0.0f, 470, calcMomentOfIntertia(470, scaleEnemy),10.f));
		addComp<Health>(enemy, Health(100));
		addComp<Enemy>(enemy, player);
		addComp<TextureRef>(enemy, TextureRef("test.png", Vec2(1.f / 16.f * 3.f, 1.f / 16.f * 15.f), Vec2(1.f / 16.f * 4.f, 1.f / 16.f * 16.f)));
		spawn(enemy);

		auto pinguin = createEnt();
		addComp<Base>(pinguin, Base(Vec2(3, 4), 0));
		addComp<Draw>(pinguin, Draw(Vec4(1, 1, 1, 0.5), Vec2(2, 3), 0.5F, Form::RECTANGLE));
		addComp<Collider>(pinguin, Collider(Vec2(2,3), Form::RECTANGLE));
		addComp<TextureRef>(pinguin, TextureRef("pingu.png"));
		addComp<PhysicsBody>(pinguin, PhysicsBody(1, 8, calcMomentOfIntertia(8, Vec2(2, 3)), 0.5));
		addComp<PhysicsBody>(pinguin);
		spawn(pinguin);*/

		Collider	wallCollider(Vec2(0.4f, 10.0f), Form::RECTANGLE);
		PhysicsBody	wallSolidBody(0.5f, 1'000'000'000'000'000.0f, calcMomentOfIntertia(1'000'000'000'000'000.0f, Vec2(0.4f, 10.0f)), 100.0f);
		Draw		wallDraw = Draw(Vec4(0, 0, 0, 1), Vec2(0.4f, 10.0f), 0.5f, Form::RECTANGLE, true);
		for (int i = 0; i < 4; i++) {
			auto wall = create();
			std::cout << "wallid: " << wall << std::endl;
			float rotation = 90.0f * i;
			addComp<Base>(wall, Base(rotate(Vec2(-5.f, 0.0f), rotation), rotation));
			addComp<Collider>(wall, wallCollider);
			addComp<PhysicsBody>(wall, wallSolidBody);
			addComp<Draw>(wall, wallDraw);
			//addComp<TextureRef>(wall, TextureRef("test.png",vec2(0,0), vec2(1,25)));
			spawn(wall);
		}

		int num = 33;
		Vec2 scale = Vec2(0.05f, 0.05f);
		Collider trashCollider = Collider(scale, Form::RECTANGLE);
		Draw trashDraw = Draw(Vec4(1.0f, 1.0f, 1.0f, 1), scale, 0.5f, Form::RECTANGLE, true);
		PhysicsBody trashSolidBody(0.9f, 1.0f, calcMomentOfIntertia(1, scale), 10.0f);
		for (int i = 0; i < num; i++) {
			if (i % 2) {
				//trashCollider.form = Form::CIRCLE;
				//trashDraw.form = Form::CIRCLE;
			}
			else {
				trashCollider.form = Form::RECTANGLE;
				trashDraw.form = Form::RECTANGLE;
			}


			Vec2 position = { static_cast<float>(rand() % 1000 / 500.0f - 1.0f) * 4.6f, static_cast<float>(rand() % 1000 / 500.0f - 1.0f) * 4.6f };
			auto trash = create();
			addComp<Base>(trash, Base(position, RotaVec2(0)));
			addComp<Movement>(trash, Movement(rand() % 1000 / 10000.0f - 0.05f, rand() % 1000 / 10000.0f - 0.05f));
			addComp<Collider>(trash, trashCollider);
			addComp<Draw>(trash, trashDraw);
			addComp<PhysicsBody>(trash, trashSolidBody);
			addComp<Health>(trash, Health(100));
			addComp<TextureRef>(trash, TextureRef("Dir.png"));
			spawn(trash);

			auto trashAss = create();
			auto cmps = viewComps(trashAss);
			cmps.add<Base>();
			cmps.add<Movement>();
			auto coll = trashCollider;
			coll.form = Form::CIRCLE;
			cmps.add<Coll>(coll);
			cmps.add<PhysicsBody>();
			auto draw = trashDraw;
			draw.form = Form::CIRCLE;
			cmps.add<Draw>(draw);
			cmps.add<TexRef>(TextureRef("Dir.png"));
			link(trashAss, trash, Vec2(0, 0.02f), 0);
			spawn(trashAss);

			trashAss = create();
			auto cmps2 = viewComps(trashAss);
			cmps2.add<Base>();
			cmps2.add<Movement>();
			coll = trashCollider;
			coll.form = Form::CIRCLE;
			cmps2.add<Coll>(coll);
			cmps2.add<PhysicsBody>();
			draw = trashDraw;
			draw.form = Form::CIRCLE;
			cmps2.add<Draw>(draw);
			cmps2.add<TexRef>(TextureRef("Dir.png"));
			link(trashAss, trash, Vec2(0, -0.02f), 0);
			spawn(trashAss);
		}

		int num2 = 0;
		Vec2 scale2 = Vec2(0.04f, 0.04f);
		Collider trashCollider2 = Collider(scale2, Form::RECTANGLE);
		Draw trashDraw2 = Draw(Vec4(1.0f, 1.0f, 1.0f, 1), scale2, 0.5f, Form::RECTANGLE, true);
		PhysicsBody trashSolidBody2(0.9f, 1'000'000'000'000'000.0f, calcMomentOfIntertia(1, scale), 10.0f);
		for (int i = 0; i < num2; i++) {

			Vec2 position2 = { static_cast<float>(rand() % 1000 / 10.0f - 50.0f) * 4.6f, static_cast<float>(rand() % 1000 / 10.0f - 50.0f) * 4.6f };
			auto trash2 = create();
			addComp<Base>(trash2, Base(position2, RotaVec2(0)));
			addComp<Collider>(trash2, trashCollider2);
			addComp<PhysicsBody>(trash2, trashSolidBody2);
			spawn(trash2);
		}
	}
}
