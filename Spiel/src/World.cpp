#include "World.hpp"
#include "Physics.hpp"

void World::loadMap(const std::string& mapname) {
	*this = World();
	if (mapname == "standart")
	{
		auto makeWall = [&](int x, int y) {
			auto wall = create();
			auto comp = componentView(wall);
			comp.add<Transform>(Transform(Vec2(x, y), 0));
			comp.add<Draw>(Draw(Vec4(1, 1, 1, 1), Vec2(1.2, 1.2), 0.45f, Form::Rectangle));
			auto coll = Collider(Vec2(1.2, 1.2), Form::Rectangle);
			coll.setIgnore(Collider::DYNAMIC);
			comp.add<Collider>(coll);
			comp.add<PhysicsBody>(PhysicsBody(0.0f, 10000000000000000000000000000000000.0f, 1000000000000000000000000000000000.0f, 1));
			spawn(wall);
		};

		this->physics.friction = 0.25f;
		//this->physics.linearEffectAccel = 3.0;
		this->physics.linearEffectDir = Vec2(0,-1);


		int const height = 80;
		int const width = 32;
		std::string map = {
			"################################"
			"#                              #"
			"#                              #"
			"#                              #"
			"#                              #"
			"#                              #"
			"#                              #"
			"#                              #"
			"#                              #"
			"#                              #"
			"#                              #"
			"#                              #"
			"#                              #"
			"#                              #"
			"#                              #"
			"#                              #"
			"#                              #"
			"#                              #"
			"#                              #"
			"#                              #"
			"#                              #"
			"#                              #"
			"#                              #"
			"#                              #"
			"#                              #"
			"#                              #"
			"#                              #"
			"#                              #"
			"#                              #"
			"#                              #"
			"#                              #"
			"#                              #"
			"#                              #"
			"#                              #"
			"#                              #"
			"#                              #"
			"#                              #"
			"#                              #"
			"#                              #"
			"#                              #"
			"#                              #"
			"#                              #"
			"#                              #"
			"#                              #"
			"#                              #"
			"#                              #"
			"#                              #"
			"#                              #"
			"#                              #"
			"#                              #"
			"#                              #"
			"#                              #"
			"#                              #"
			"#                              #"
			"#                              #"
			"#                              #"
			"#                              #"
			"#                              #"
			"#                              #"
			"#                   #######    #"
			"#                              #"
			"#                              #"
			"#                              #"
			"#                              #"
			"#                      ##      #"
			"#                              #"
			"#                              #"
			"#                              #"
			"#                              #"
			"#                         ###  #"
			"#                              #"
			"#                              #"
			"#                              #"
			"#                    ###       #"
			"#                              #"
			"#                              #"
			"#                              #"
			"#                              #"
			"#                              #"
			"################################"
		};

		for (int vert = 0; vert < height; vert++) {
			for (int hor = 0; hor < width; hor++) {
				if (map.at(vert * width + hor) == '#') {
					//for (int i = 0; i < 100; i++)
						makeWall(hor, height - vert);
				}
			}
		}

		Vec2 scalePlayer(1, 1);
		auto player = create();
		auto cmps = componentView(player);
		cmps.add<Transform>(Transform(Vec2(2,12),0));
		auto colliderPlayer = Collider(Vec2(0.4,0.7), Form::Rectangle);
		colliderPlayer.extraColliders.push_back(CompountCollider(Vec2(1, 1)*0.4, Vec2(0,0.35), RotaVec2(0), Form::Circle));
		colliderPlayer.extraColliders.push_back(CompountCollider(Vec2(0.3, 0.2), Vec2(0.2, -0.3), RotaVec2(305.0f), Form::Rectangle));
		colliderPlayer.extraColliders.push_back(CompountCollider(Vec2(0.3, 0.2), Vec2(-0.2, -0.3), RotaVec2(55.0f), Form::Rectangle));
		cmps.add(colliderPlayer);
		cmps.add(PhysicsBody(0.0, 25.0f, calcMomentOfIntertia(25.0f, scalePlayer), 0.9f));
		cmps.add<Movement>();
		cmps.add(Draw(Vec4(1, 1, 1, 1), scalePlayer, 0.4, Form::Rectangle));
		cmps.add(TextureRef2("bitch.png"));
		cmps.add<Player>();
		identify(player);
		spawn(player);

		Vec2 scaleBox(1, 1);
		auto box = create();
		auto cmpsBox = componentView(box);
		cmpsBox.add<Transform>(Transform(Vec2(2, 2), 0));
		auto colliderBox = Collider(scalePlayer, Form::Rectangle);
		colliderBox.groupMask |= CollisionGroup<1>::mask;
		cmpsBox.add(colliderBox);
		cmpsBox.add(PhysicsBody(0.0, 5.0f, calcMomentOfIntertia(5.0f, scaleBox), 0.9f));
		cmpsBox.add<Movement>();
		cmpsBox.add(Draw(Vec4(1, 1, 1, 1), scaleBox, 0.4, Form::Rectangle));
		cmpsBox.add(TextureRef2("Dir.png"));
		cmpsBox.add(Health(100));
		spawn(box);

		for (int i = 0; i < 300'000; ++i) {
			auto ent = create();
			addComp(ent, Transform());
			addComp(ent, Draw(Vec4(1, 1, 1, 1), scaleBox, 0.4, Form::Rectangle));
			spawn(ent);
		}

		Vec2 scale = Vec2(0.3f, 0.3f);
		Form form = Form::Circle;
		Collider trashCollider = Collider(scale, form); 
		PhysicsBody trashSolidBody = PhysicsBody(0.0f, 4.5f, calcMomentOfIntertia(4.5, scale),0.9f);
		for (int i = 0; i < 10000; i ++) {
			//if (i % 2) {
			//	form = Form::Rectangle;
			//	scale = { 0.2f, 0.4f }; 
			//	trashCollider = Collider(scale, form);
			//}
			//else {
				form = Form::Circle;
				scale = { 0.1f, 0.1f }; 
				trashCollider = Collider(scale, form);
			//}
			trashCollider.form = form;
			Vec4 color = Vec4(rand() % 1000 / 1000.0f, rand() % 1000 / 1000.0f, rand() % 1000 / 1000.0f, 1);
			//Vec2 position = Vec2(5, 1.6 + i * 0.301f);
			Vec2 position = { static_cast<float>(rand() % 1001 / 300.0f) * 4.6f + 5.5f, static_cast<float>(rand() % 1000 / 100.0f) * 4.6f + 5.5f };
			auto trash = create();
			addComp(trash, Transform(position, RotaVec2(0)));
			addComp(trash, Movement());
			addComp(trash, trashCollider); 
			addComp(trash, Draw(color, scale, 0.5f, form));
			addComp(trash, trashSolidBody);
			addComp(trash, Health(100));
			addComp(trash, TextureRef2("Dir.png"));
			spawn(trash);
		}

		auto spawner = create();
		auto cmps2 = componentView(spawner);
		cmps2.add<Transform>(Transform(Vec2(20, 40), 0));
		cmps2.add<Collider>(Collider(Vec2(0.3, 0.3), Form::Circle));
		cmps2.add<PhysicsBody>(PhysicsBody(0.0f, 100000000000000000000000000000000.0f,10000000000000000000000000000000000.0f,0));
		cmps2.add<SpawnerComp>();
		spawn(spawner);
		
		auto sucker = create();
		auto cmps3 = componentView(sucker);
		cmps3.add<Transform>(Transform(Vec2(20, 2), 0));
		auto coll = Collider(Vec2(7, 7), Form::Circle);
		cmps3.add<Collider>(coll);
		cmps3.add<Draw>(Draw(Vec4(0, 0, 1, 1), Vec2(7, 7), 0.4f, Form::Circle));
		auto suckerCmd = SuckerComp();
		suckerCmd.spawner = identify(spawner);
		cmps3.add<SuckerComp>(suckerCmd);
		spawn(sucker);
	}

	if (mapname == "debug") {
		auto rect = create();
		
		addComp(rect, Transform{ { 0, 0 } });
		addComp(rect, Draw{ {1.0f,0.5f,0.0f,1.0f}, {1.0f, 1.0f}, 0.0f, Form::Rectangle });
		spawn(rect);


	}
}

void World::saveMap(const std::string& mapname)
{
}