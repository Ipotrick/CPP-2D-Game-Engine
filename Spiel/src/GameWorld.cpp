#include "GameWorld.hpp"
#include "Physics.hpp"

void World::loadMap(std::string mapname_) {
	std::ifstream mapData(mapname_);
	if (mapData.good()) {
	}
	else
	{
		auto makeWall = [&](int x, int y) {
			auto wall = index_create();
			auto comp = viewComps(wall);
			comp.add<Base>(Base(Vec2(x, y), 0));
			comp.add<Draw>(Draw(Vec4(1, 1, 1, 1), Vec2(1, 1), 0.45f, Form::Rectangle));
			comp.add<Collider>(Collider(Vec2(1, 1), Form::Rectangle));
			comp.add<PhysicsBody>(PhysicsBody(0.0f, 10000000000000000000000000000000000.0f, 1000000000000000000000000000000000.0f, 30));
			spawn(wall);
		};

		this->physics.friction = 0.01f;
		this->physics.linearEffectAccel = 3.8;
		this->physics.linearEffectDir = Vec2(0,-1);

		int const height = 16*16;
		int const width = 64+4;
		std::string map = {
			"################################    ################################"
			"#                              #    #                              #"
			"#                              #    #                              #"
			"#                              #    #                              #"
			"#        ###########           #    #                              #"
			"#        #                     #    #                              #"
			"#        #                     ######                              #"
			"#        ##########                                                #"
			"#                 #                                                #"
			"#                 #            ######                              #"
			"#                 #            #    #                              #"
			"#                 #            #    #                              #"
			"#                 #            #    #                              #"
			"#                              #    #                              #"
			"#                              #    #                              #"
			"################     ###########    ################################"
			"################     ###########    ################################"
			"#                              #    #                              #"
			"#                              #    #                              #"
			"#                              #    #                              #"
			"#        ###########           #    #                              #"
			"#        #                     #    #                              #"
			"#        #                     ######                              #"
			"#        ##########                                                #"
			"#                 #                                                #"
			"#                 #            ######                              #"
			"#                 #            #    #                              #"
			"#                 #            #    #                              #"
			"#                 #            #    #                              #"
			"#                              #    #                              #"
			"#                              #    #                              #"
			"###################    #########    ################################"
			"###################    #########    ################################"
			"#                              #    #                              #"
			"#                              #    #                              #"
			"#                              #    #                              #"
			"#        ###########           #    #                              #"
			"#        #                     #    #                              #"
			"#        #                     ######                              #"
			"#        ##########                                                #"
			"#                 #                                                #"
			"#                 #            ######                              #"
			"#                 #            #    #                              #"
			"#                 #            #    #                              #"
			"#                 #            #    #                              #"
			"#                              #    #                              #"
			"#                              #    #                              #"
			"#############     ##############    ################################"
			"#############     ##############    ################################"
			"#                              #    #                              #"
			"#                              #    #                              #"
			"#                              #    #                              #"
			"#        ###########           #    #                              #"
			"#        #                     #    #                              #"
			"#        #                     ######                              #"
			"#        ##########                                                #"
			"#                 #                                                #"
			"#                 #            ######                              #"
			"#                 #            #    #                              #"
			"#                 #            #    #                              #"
			"#                 #            #    #                              #"
			"#                              #    #                              #"
			"#                              #    #                              #"
			"################     ###########    ################################"
			"################     ###########    ################################"
			"#                              #    #                              #"
			"#                              #    #                              #"
			"#                              #    #                              #"
			"#        ###########           #    #                              #"
			"#        #                     #    #                              #"
			"#        #                     ######                              #"
			"#        ##########                                                #"
			"#                 #                                                #"
			"#                 #            ######                              #"
			"#                 #            #    #                              #"
			"#                 #            #    #                              #"
			"#                 #            #    #                              #"
			"#                              #    #                              #"
			"#                              #    #                              #"
			"#############     ##############    ################################"
			"#############     ##############    ################################"
			"#                              #    #                              #"
			"#                              #    #                              #"
			"#                              #    #                              #"
			"#        ###########           #    #                              #"
			"#        #                     #    #                              #"
			"#        #                     ######                              #"
			"#        ##########                                                #"
			"#                 #                                                #"
			"#                 #            ######                              #"
			"#                 #            #    #                              #"
			"#                 #            #    #                              #"
			"#                 #            #    #                              #"
			"#                              #    #                              #"
			"#                              #    #                              #"
			"###############           ######    ################################"
			"###############           ######    ################################"
			"#                              #    #                              #"
			"#                              #    #                              #"
			"#                              #    #                              #"
			"#        ###########           #    #                              #"
			"#        #                     #    #                              #"
			"#        #                     ######                              #"
			"#        ##########                                                #"
			"#                 #                                                #"
			"#                 #            ######                              #"
			"#                 #            #    #                              #"
			"#                 #            #    #                              #"
			"#                 #            #    #                              #"
			"#                              #    #                              #"
			"#                              #    #                              #"
			"#####################   ########    ################################"
			"#####################   ########    ################################"
			"#                              #    #                              #"
			"#                              #    #                              #"
			"#                              #    #                              #"
			"#        ###########           #    #                              #"
			"#        #                     #    #                              #"
			"#        #                     ######                              #"
			"#        ##########                                                #"
			"#                 #                                                #"
			"#                 #            ######                              #"
			"#                 #            #    #                              #"
			"#                 #            #    #                              #"
			"#                 #            #    #                              #"
			"#                              #    #                              #"
			"#                              #    #                              #"
			"################################    #########  #####################"
			"################################    #########  #####################"
			"#                              #    #                              #"
			"#                              #    #                              #"
			"#                              #    #                              #"
			"#        ###########           #    #                              #"
			"#        #                     #    #                              #"
			"#        #                     ######                              #"
			"#        ##########                                                #"
			"#                 #                                                #"
			"#                 #            ######                              #"
			"#                 #            #    #                              #"
			"#                 #            #    #                              #"
			"#                 #            #    #                              #"
			"#                              #    #                              #"
			"#                              #    #                              #"
			"#####################  #########    ################################"
			"#####################  #########    ################################"
			"#                              #    #                              #"
			"#                              #    #                              #"
			"#                              #    #                              #"
			"#        ###########           #    #                              #"
			"#        #                     #    #                              #"
			"#        #                     ######                              #"
			"#        ##########                                                #"
			"#                 #                                                #"
			"#                 #            ######                              #"
			"#                 #            #    #                              #"
			"#                 #            #    #                              #"
			"#                 #            #    #                              #"
			"#                              #    #                              #"
			"#                              #    #                              #"
			"################################    #########  #####################"
			"################################    #########  #####################"
			"#                              #    #                              #"
			"#                              #    #                              #"
			"#                              #    #                              #"
			"#        ###########           #    #                              #"
			"#        #                     #    #                              #"
			"#        #                     ######                              #"
			"#        ##########                                                #"
			"#                 #                                                #"
			"#                 #            ######                              #"
			"#                 #            #    #                              #"
			"#                 #            #    #                              #"
			"#                 #            #    #                              #"
			"#                              #    #                              #"
			"#                              #    #                              #"
			"##############  ################    ################################"
			"##############  ################    ################################"
			"#                              #    #                              #"
			"#                              #    #                              #"
			"#                              #    #                              #"
			"#        ###########           #    #                              #"
			"#        #                     #    #                              #"
			"#        #                     ######                              #"
			"#        ##########                                                #"
			"#                 #                                                #"
			"#                 #            ######                              #"
			"#                 #            #    #                              #"
			"#                 #            #    #                              #"
			"#                 #            #    #                              #"
			"#                              #    #                              #"
			"#                              #    #                              #"
			"########################  ######    ################################"
			"########################  ######    ################################"
			"#                              #    #                              #"
			"#                              #    #                              #"
			"#                              #    #                              #"
			"#        ###########           #    #                              #"
			"#        #                     #    #                              #"
			"#        #                     ######                              #"
			"#        ##########                                                #"
			"#                 #                                                #"
			"#                 #            ######                              #"
			"#                 #            #    #                              #"
			"#                 #            #    #                              #"
			"#                 #            #    #                              #"
			"#                              #    #                              #"
			"#                              #    #                              #"
			"##################  ############    ################################"
			"##################  ############    ################################"
			"#                              #    #                              #"
			"#                              #    #                              #"
			"#                              #    #                              #"
			"#        ###########           #    #                              #"
			"#        #                     #    #                              #"
			"#        #                     ######                              #"
			"#        ##########                                                #"
			"#                 #                                                #"
			"#                 #            ######                              #"
			"#                 #            #    #                              #"
			"#                 #            #    #                              #"
			"#                 #            #    #                              #"
			"#                              #    #                              #"
			"#                              #    #                              #"
			"####################  ##########    ################################"
			"####################  ##########    ################################"
			"#                              #    #                              #"
			"#                              #    #                              #"
			"#                              #    #                              #"
			"#        ###########           #    #                              #"
			"#        #                     #    #                              #"
			"#        #                     ######                              #"
			"#        ##########                                                #"
			"#                 #                                                #"
			"#                 #            ######                              #"
			"#                 #            #    #                              #"
			"#                 #            #    #                              #"
			"#                 #            #    #                              #"
			"#                              #    #                              #"
			"#                              #    #                              #"
			"#####################  #########    ################################"
			"#####################  #########    ################################"
			"#                              #    #                              #"
			"#                              #    #                              #"
			"#                              #    #                              #"
			"#        ###########           #    #                              #"
			"#        #                     #    #                              #"
			"#        #                     ######                              #"
			"#        ##########                                                #"
			"#                 #                                                #"
			"#                 #            ######                              #"
			"#                 #            #    #                              #"
			"#                 #            #    #                              #"
			"#                 #            #    #                              #"
			"#                              #    #                              #"
			"#                              #    #                              #"
			"################################    ################################"
		};

		for (int vert = 0; vert < height; vert++) {
			for (int hor = 0; hor < width; hor++) {
				if (map.at(vert * width + hor) == '#') 
					makeWall(hor, vert);
			}
		}

		Vec2 scalePlayer(1, 1);
		auto player = create();
		auto cmps = viewComps(player);
		cmps.add<Base>(Base(Vec2(5,5),0));
		auto colliderPlayer = Collider(scalePlayer, Form::Rectangle);
		cmps.add<Collider>(colliderPlayer);
		cmps.add<PhysicsBody>(PhysicsBody(0.5, 5.0f, /*calcMomentOfIntertia(1.0f, scalePlayer)*/ 1000000000000000000000000.0f, 1.0f));
		cmps.add<Movement>();
		cmps.add<Draw>(Draw(Vec4(1, 1, 1, 1), scalePlayer, 0.5, Form::Rectangle));
		cmps.add<Player>();
		spawn(player);

		int num = 10000;// 250'000;
		for (int i = 0; i < num; i++) {
			auto ent = index_create();
			auto c = viewComps(ent);
			c.add<Base>(Base(Vec2(rand()%10000/100.0f, rand()%10000/100.0f)));
			c.add<Draw>(Draw(Vec4(rand()%1000 / 1000.0f, rand() % 1000 / 1000.0f, rand() % 1000 / 1000.0f, 1), Vec2(1, 1), 0.3f, Form::Rectangle));
			spawn(ent);
		}

		Vec2 scale = Vec2(0.1f, 0.1f);
		Collider trashCollider = Collider(scale, Form::Circle);
		PhysicsBody trashSolidBody(0.9f, 1.2f, calcMomentOfIntertia(1.2, scale), 30.0f);
		for (int i = 0; i < 4000; i ++) {
			Vec4 color = Vec4(rand() % 1000 / 1000.0f, rand() % 1000 / 1000.0f, rand() % 1000 / 1000.0f, 1);
			Vec2 position = { static_cast<float>(rand() % 1001 / 500.0f - 1.0f) * 4.6f + 5.5f , static_cast<float>(rand() % 1000 / 500.0f - 1.0f) * 4.6f + 5.5f };
			auto trash = index_create();
			addComp<Base>(trash, Base(position, RotaVec2(0)));
			addComp<Movement>(trash, Movement(rand() % 1000 / 10000.0f - 0.05f, rand() % 1000 / 10000.0f - 0.05f));
			addComp<Collider>(trash, trashCollider);
			addComp<Draw>(trash, Draw(color, scale, 0.5f, Form::Circle, true));
			addComp<PhysicsBody>(trash, trashSolidBody);
			addComp<Health>(trash, Health(100));
			//addComp(trash, TexRef("Dir.png"));
			spawn(trash);

			//auto trashDraw = Draw(color, scale, 0.5f, Form::Circle, true);
			//auto trashAss = index_create();
			//auto cmps = viewComps(trashAss);
			//cmps.add<Base>(Base(position));
			//cmps.add<Movement>();
			//auto coll = trashCollider;
			//coll.form = Form::Circle;
			//cmps.add<Coll>(coll);
			//cmps.add<PhysicsBody>();
			//auto draw = trashDraw;
			//draw.form = Form::Circle;
			//cmps.add<Draw>(draw);
			//link(trashAss, trash, Vec2(0, 0.05f), 0);
			//spawn(trashAss);
			//
			//trashAss = index_create();
			//auto cmps2 = viewComps(trashAss);
			//cmps2.add<Base>(Base(position));
			//cmps2.add<Movement>();
			//coll = trashCollider;
			//coll.form = Form::Circle;
			//cmps2.add<Coll>(coll);
			//cmps2.add<PhysicsBody>();
			//draw = trashDraw;
			//draw.form = Form::Circle;
			//cmps2.add<Draw>(draw);
			//link(trashAss, trash, Vec2(0, -0.05f), 0);
			//spawn(trashAss);
		}
		/*
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
		}*/

	}
}
