#include "World.hpp"
#include "Physics.hpp"

void World::loadMap(std::string mapname_) {
	*this = World();
	if (mapname_ == "standart")
	{
		auto makeWall = [&](int x, int y) {
			auto wall = create();
			auto comp = componentView(wall);
			comp.add<Base>(Base(Vec2(x, y), 0));
			comp.add<Draw>(Draw(Vec4(1, 1, 1, 1), Vec2(1.2, 1.2), 0.45f, Form::Rectangle));
			auto coll = Collider(Vec2(1.2, 1.2), Form::Rectangle);
			coll.setIgnore(Collider::DYNAMIC);
			comp.add<Collider>(coll);
			comp.add<PhysicsBody>(PhysicsBody(0.0f, 10000000000000000000000000000000000.0f, 1000000000000000000000000000000000.0f, 1));
			spawn(wall);
		};

		this->physics.friction = 0.25f;
		this->physics.linearEffectAccel = 2.3;
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
		auto player = idCreate();
		auto cmps = componentView(player);
		cmps.add<Base>(Base(Vec2(2,12),0));
		auto colliderPlayer = Collider(Vec2(0.4,0.7), Form::Rectangle);
		colliderPlayer.extraColliders.push_back(CompountCollider(Vec2(1, 1)*0.4, Vec2(0,0.35), RotaVec2(0), Form::Circle));
		colliderPlayer.extraColliders.push_back(CompountCollider(Vec2(0.3, 0.2), Vec2(0.2, -0.3), RotaVec2(305.0f), Form::Rectangle));
		colliderPlayer.extraColliders.push_back(CompountCollider(Vec2(0.3, 0.2), Vec2(-0.2, -0.3), RotaVec2(55.0f), Form::Rectangle));
		cmps.add(colliderPlayer);
		cmps.add(PhysicsBody(0.0, 25.0f, calcMomentOfIntertia(25.0f, scalePlayer), 0.9f));
		cmps.add<Movement>();
		cmps.add(Draw(Vec4(1, 1, 1, 1), scalePlayer, 0.4, Form::Rectangle));
		cmps.add(TexRef(texture.getId("bitch.png")));
		cmps.add<Player>();
		spawn(player);

		Vec2 scaleBox(1, 1);
		auto box = idCreate();
		auto cmpsBox = componentView(box);
		cmpsBox.add<Base>(Base(Vec2(2, 2), 0));
		auto colliderBox = Collider(scalePlayer, Form::Rectangle);
		colliderBox.groupMask |= CollisionGroup<1>::mask;
		cmpsBox.add(colliderBox);
		cmpsBox.add(PhysicsBody(0.0, 5.0f, calcMomentOfIntertia(5.0f, scaleBox), 0.9f));
		cmpsBox.add<Movement>();
		cmpsBox.add(Draw(Vec4(1, 1, 1, 1), scaleBox, 0.4, Form::Rectangle));
		spawn(box);

		int num = 100'000;// 250'000;
		for (int i = 0; i < num; i++) {
			auto ent = create();
			auto c = componentView(ent);
			c.add<Base>(Base(Vec2(rand()%10000/10.0f, rand()%10000/10.0f)));
			c.add<Draw>(Draw(Vec4(rand()%1000 / 1000.0f, rand() % 1000 / 1000.0f, rand() % 1000 / 1000.0f, 1), Vec2(1, 1), 0.1f, Form::Rectangle));
			c.add<Tester>();
			spawn(ent);
		}

		Vec2 scale = Vec2(0.2f, 0.2f);
		Form form = Form::Circle;
		Collider trashCollider = Collider(scale, form);
		PhysicsBody trashSolidBody = PhysicsBody(0.0f, 0.5f, calcMomentOfIntertia(0.5, scale),0.9f);
		for (int i = 0; i < 6000; i ++) {
			Vec4 color = Vec4(rand() % 1000 / 1000.0f, rand() % 1000 / 1000.0f, rand() % 1000 / 1000.0f, 1);
			//Vec2 position = Vec2(5, 1.6 + i * 0.301f);
			Vec2 position = { static_cast<float>(rand() % 1001 / 300.0f) * 4.6f + 5.5f, static_cast<float>(rand() % 1000 / 100.0f) * 4.6f + 5.5f };
			auto trash = create();
			addComp(trash, Base(position, RotaVec2(0)));
			addComp(trash, Movement());
			addComp(trash, trashCollider); 
			addComp(trash, Draw(color, scale, 0.5f, form));
			addComp(trash, trashSolidBody);
			addComp(trash, Health(100));
			addComp(trash, TextureRef(texture.getId("Dir.png")));
			spawn(trash);
		}

		auto spawner = idCreate();
		auto cmps2 = componentView(spawner);
		cmps2.add<Base>(Base(Vec2(20, 40), 0));
		cmps2.add<Collider>(Collider(Vec2(0.3, 0.3), Form::Circle));
		cmps2.add<PhysicsBody>(PhysicsBody(0.0f, 100000000000000000000000000000000.0f,10000000000000000000000000000000000.0f,0));
		cmps2.add<SpawnerComp>();
		spawn(spawner);
		
		auto sucker = create();
		auto cmps3 = componentView(sucker);
		cmps3.add<Base>(Base(Vec2(20, 2), 0));
		auto coll = Collider(Vec2(7, 7), Form::Circle);
		cmps3.add<Collider>(coll);
		cmps3.add<Draw>(Draw(Vec4(0, 0, 1, 1), Vec2(7, 7), 0.4f, Form::Circle));
		auto suckerCmd = SuckerComp();
		suckerCmd.spawner = spawner;
		cmps3.add<SuckerComp>(suckerCmd);
		spawn(sucker);
	}
	else if (mapname_ == "uitest") {
		this->physics.linearEffectDir = Vec2(0, -1);
	}
	else {
		std::ifstream ifs(mapname_, std::ios::binary);
		if (ifs.good()) {
			boost::archive::binary_iarchive oa(ifs);
			oa >> *this;
		}
		else {
			loadMap("standart");
		}
	}
}

void World::saveMap(std::string filename)
{
	defragment(DefragMode::COMPLETE);
	std::ofstream ofs(filename, std::ios::binary);
	if (ofs.good()) {
		{
			boost::archive::binary_oarchive oa(ofs);
			oa << *this;
		}
	}
}
