#include "GameWorld.h"
#include "Physics.h"

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

		this->physics.friction = 1.0f;

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
			"################################    ################################"
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
			"################################    ################################"
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
			"################################    ################################"
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
			"################################    ################################"
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
			"################################    ################################"
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
			"################################    ################################"
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
			"################################    ################################"
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
			"################################    ################################"
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
			"################################    ################################"
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
			"################################    ################################"
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
			"################################    ################################"
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
			"################################    ################################"
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
			"################################    ################################"
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
			"################################    ################################"
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
			"################################    ################################"
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
		cmps.add<Collider>(Collider(scalePlayer, Form::Rectangle));
		cmps.add<PhysicsBody>(PhysicsBody(0.5, 10.0f, /*calcMomentOfIntertia(1.0f, scalePlayer)*/ 1000000000000000000000000.0f, 1.0f));
		cmps.add<Movement>();
		cmps.add<Draw>(Draw(Vec4(1, 1, 1, 1), scalePlayer, 0.5, Form::Rectangle));
		cmps.add<Player>();
		spawn(player);


		int num = 0;// 250'000;
		for (int i = 0; i < num; i++) {
			auto ent = index_create();
			auto c = viewComps(ent);
			c.add<Base>();
			c.add<Movement>();
			c.add<Draw>(Draw(Vec4(1, 1, 1, 1), Vec2(0, 0), 0.4f, Form::Rectangle));
			spawn(ent);
		}


		/*
		Vec2 scalePlayer = { 0.4f, 0.8f };
		physics.friction = 0.16f;
		//physics.linearEffectDir = Vec2(0, -1);
		//physics.linearEffectAccel = 1.f;

		entity_handle player = create();
		addComp<Base>(player, Base({ 0,0 }, 0));
		addComp<Movement>(player, Movement(0.0f, 0.0f));
		addComp<Draw>(player, Draw(Vec4(1, 1, 1, 1), scalePlayer, 0.6f, Form::RECTANGLE));
		addComp<Collider>(player, Collider(scalePlayer, Form::RECTANGLE));
		addComp<PhysicsBody>(player, PhysicsBody(0.1f, 60, calcMomentOfIntertia(60, scalePlayer), 100));
		addComp<TextureRef>(player, TextureRef("Dir.png"));
		addComp<Player>(player, Player());
		spawn(player);

		auto slave = create();
		addComp<Base>(slave);
		addComp<Movement>(slave);
		addComp<PhysicsBody>(slave);
		addComp<Collider>(slave, Collider({ scalePlayer.x * 1 / sqrtf(2.0f) }, Form::RECTANGLE));
		addComp<Draw>(slave, Draw(Vec4(0, 0, 0, 1), { scalePlayer.x * 1 / sqrtf(2.0f) }, 0.6f, Form::RECTANGLE));
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
		}*/

		Vec2 scale = Vec2(0.1f, 0.1f);
		Collider trashCollider = Collider(scale, Form::Rectangle);
		Draw trashDraw = Draw(Vec4(1.0f, 1.0f, 1.0f, 1), scale, 0.5f, Form::Rectangle, true);
		PhysicsBody trashSolidBody(0.9f, 0.8f, calcMomentOfIntertia(1, scale), 10.0f);
		for (int i = 0; i < 3000; i++) {

			Vec2 position = { static_cast<float>(rand() % 1000 / 500.0f - 1.0f) * 4.6f + 5 , static_cast<float>(rand() % 1000 / 500.0f - 1.0f) * 4.6f + 5 };
			auto trash = index_create();
			addComp<Base>(trash, Base(position, RotaVec2(0)));
			addComp<Movement>(trash, Movement(rand() % 1000 / 10000.0f - 0.05f, rand() % 1000 / 10000.0f - 0.05f));
			addComp<Collider>(trash, trashCollider);
			addComp<Draw>(trash, trashDraw);
			addComp<PhysicsBody>(trash, trashSolidBody);
			addComp<Health>(trash, Health(100));
			spawn(trash);

			auto trashAss = index_create();
			auto cmps = viewComps(trashAss);
			cmps.add<Base>(Base(position));
			cmps.add<Movement>();
			auto coll = trashCollider;
			coll.form = Form::Circle;
			cmps.add<Coll>(coll);
			cmps.add<PhysicsBody>();
			auto draw = trashDraw;
			draw.form = Form::Circle;
			cmps.add<Draw>(draw);
			link(trashAss, trash, Vec2(0, 0.05f), 0);
			spawn(trashAss);
			
			trashAss = index_create();
			auto cmps2 = viewComps(trashAss);
			cmps2.add<Base>(Base(position));
			cmps2.add<Movement>();
			coll = trashCollider;
			coll.form = Form::Circle;
			cmps2.add<Coll>(coll);
			cmps2.add<PhysicsBody>();
			draw = trashDraw;
			draw.form = Form::Circle;
			cmps2.add<Draw>(draw);
			link(trashAss, trash, Vec2(0, -0.05f), 0);
			spawn(trashAss);
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
