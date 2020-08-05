#include "GameWorld.hpp"
#include "Physics.hpp"

void World::loadMap(std::string mapname_) {
	std::ifstream mapData(mapname_);
	if (mapData.good()) {
	}
	else if (mapname_ == "ballstest")
	{
		auto makeWall = [&](int x, int y) {
			auto wall = index_create();
			auto comp = viewComps(wall);
			comp.add<Base>(Base(Vec2(x, y), 0));
			comp.add<Draw>(Draw(Vec4(1, 1, 1, 1), Vec2(1.2, 1.2), 0.45f, Form::Rectangle));
			comp.add<Collider>(Collider(Vec2(1.2, 1.2), Form::Rectangle));
			comp.add<PhysicsBody>(PhysicsBody(0.0f, 10000000000000000000000000000000000.0f, 1000000000000000000000000000000000.0f, 30));
			spawn(wall);
		};

		this->physics.friction = 0.25f;
		//this->physics.linearEffectAccel = 4;
		this->physics.linearEffectDir = Vec2(0,-1);

		int const height = 80;
		int const width =32;
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
			"#####                     ######"
			"#                              #"
			"#                              #"
			"#                              #"
			"#                              #"
			"#                              #"
			"#         ##          ##       #"
			"#           ##########         #"
			"#                              #"
			"#                              #"
			"#                              #"
			"#                              #"
			"#                              #"
			"##                            ##"
			"#                              #"
			"#                              #"
			"#                              #"
			"#                              #"
			"#                              #"
			"#                              #"
			"#                              #"
			"################################"
		};

		const int buu = 3;

		for (int vert = 0; vert < height; vert++) {
			for (int hor = 0; hor < width; hor++) {
				if (map.at(vert * width + hor) == '#') 
					makeWall(hor, height - vert);
			}
		}

		Vec2 scalePlayer(1, 1);
		auto player = id_create();
		auto cmps = viewComps(player);
		cmps.add<Base>(Base(Vec2(5,5),0));
		auto colliderPlayer = Collider(scalePlayer, Form::Rectangle);
		colliderPlayer.collisionMaskSelf |= CollisionGroup<1>::mask;
		cmps.add(colliderPlayer);
		cmps.add(PhysicsBody(0.5, 50.0f, /*calcMomentOfIntertia(1.0f, scalePlayer)*/ 1000000000000000000000000.0f, 1.0f));
		cmps.add<Movement>();
		cmps.add(Draw(Vec4(1, 1, 1, 1), scalePlayer, 0.4, Form::Rectangle));
		cmps.add<Player>();
		spawn(player);

		int num = 1'000;// 250'000;
		for (int i = 0; i < num; i++) {
			auto ent = index_create();
			auto c = viewComps(ent);
			c.add<Base>(Base(Vec2(rand()%10000/100.0f, rand()%10000/100.0f)));
			c.add<Draw>(Draw(Vec4(rand()%1000 / 1000.0f, rand() % 1000 / 1000.0f, rand() % 1000 / 1000.0f, 1), Vec2(1, 1), 0.1f, Form::Rectangle));
			spawn(ent);
		}

		Vec2 scale = Vec2(0.2f, 0.2f);
		Collider trashCollider = Collider(scale, Form::Circle);
		PhysicsBody trashSolidBody(0.7f, 1.2f, calcMomentOfIntertia(1.2, scale), 2.0f);
		for (int i = 0; i < 20000; i ++) {
			Vec4 color = Vec4(rand() % 1000 / 1000.0f, rand() % 1000 / 1000.0f, rand() % 1000 / 1000.0f, 1);
			Vec2 position = { static_cast<float>(rand() % 1001 / 300.0f) * 4.6f + 5.5f , static_cast<float>(rand() % 1000 / 100.0f) * 4.6f + 5.5f };
			auto trash = index_create();
			addComp(trash, Base(position, RotaVec2(0)));
			addComp(trash, Movement(rand() % 1000 / 10000.0f - 0.05f, rand() % 1000 / 10000.0f - 0.05f));
			addComp(trash, trashCollider); 
			addComp(trash, Draw(color, scale, 0.5f, Form::Circle, true));
			addComp(trash, trashSolidBody);
			addComp(trash, Health(100));
			spawn(trash);

			//auto trash2 = index_create();
			//addComp(trash2, Base(position, RotaVec2(0)));
			//addComp(trash2, Movement(rand() % 1000 / 10000.0f - 0.05f, rand() % 1000 / 10000.0f - 0.05f));
			//addComp(trash2, Collider(scale, Form::Rectangle));
			//addComp(trash2, Draw(color, scale, 0.5f, Form::Rectangle, true));
			//addComp(trash2, trashSolidBody);
			//addComp(trash2, Health(100));
			//link(trash2, trash, Vec2(0, 0.1), 0);
			//spawn(trash2);
		}

		auto spawner = id_create();
		auto cmps2 = viewComps(spawner);
		cmps2.add<Base>(Base(Vec2(20, 40), 0));
		cmps2.add<Collider>(Collider(Vec2(3.3, 3.3), Form::Circle));
		cmps2.add<PhysicsBody>(PhysicsBody(0.9f, 100000000000000000000000000000000.0f,10000000000000000000000000000000000.0f,0));
		cmps2.add<SpawnerComp>();
		spawn(spawner);
		
		auto sucker = id_create();
		auto cmps3 = viewComps(sucker);
		cmps3.add<Base>(Base(Vec2(20, 2), 0));
		auto coll = Collider(Vec2(6, 6), Form::Circle);
		cmps3.add<Collider>(coll);
		cmps3.add<Draw>(Draw(Vec4(0, 0, 1, 1), Vec2(7, 7), 0.4f, Form::Circle));
		auto suckerCmd = SuckerComp();
		suckerCmd.spawner = spawner;
		cmps3.add<SuckerComp>(suckerCmd);
		spawn(sucker);
	}
	else if ("uitest") {
		this->physics.linearEffectDir = Vec2(0, -1);
	}
}
