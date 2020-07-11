#include "Game.h"
#include "htmlCompiler.h"

Game::Game() : 
	Engine(world,"Test", 1600, 900),
	playerScript{ *this },
	healthScript{ *this },
	ageScript   { *this },
	bulletScript{ *this },
	particleScript{ *this },
	dummyScript{ *this }
{
	auto size = getWindowSize();
	camera.frustumBend = (Vec2(1 / getWindowAspectRatio(), 1.0f));

	auto cursor = world.createIDX();
	Vec2 cursorScale = { 0.1f,0.1f };
	world.addComp<Base>(cursor);
	Collider cursorCollider(cursorScale, Form::RECTANGLE, true);
	world.addComp<Collider>(cursor, cursorCollider);
	//Draw cursorDraw(vec4(1, 0, 0, 1), cursorScale, 0.6f, Form::CIRCLE);
	//world.addComp<Draw>(cursor, cursorDraw);
	world.spawn(cursor);
	cursorID = world.identify(cursor);
}

void Game::create() {
	camera.zoom = 1 / 3.5f;
	world.loadMap("shit");

	{
		float angle = 33.19f;
		RotaVec2 rotaVec(angle);
		Vec2 baseVec = rotate({ 1, 0 }, 42.7);
		baseVec = rotate(rotate(baseVec, rotaVec), -42.7);
		std::cout << "angle: " << angle << " translated angle: " << getRotation(baseVec) << std::endl;
	}
	{
		for (auto ent : world.viewIDX<Player>()) {
			std::cout << "ding" << std::endl;
			auto id = world.identify(ent);
			bool hasID = world.hasID(ent);
			std::cout << "ent: " << ent << " id: " << id.id << " hasID: " << hasID << std::endl;
		}
		auto newent = world.createIDX();
		auto id = world.identify(newent);
		bool hasIDafter = world.hasID(newent);
		std::cout << "newent id: " << id.id << " hasIDafter: " << hasIDafter << std::endl;
		world.destroy(newent);

		auto newent2 = world.createIDX();
		bool hasIDbefore = world.hasID(newent2);
		auto id2 = world.identify(newent2);
		std::cout << "newent2 id: " << id.id << " hasIDbefore: " << hasIDbefore << std::endl;
	}
	world.setDefragMode(World::DefragMode::NONE);
}

void Game::update(World& world, float deltaTime) {
	//submitDrawable(Drawable(++freeDrawableID, Vec2(0, 0), 0, Vec2(2,2), Vec4(1, 1, 1, 1), Form::RECTANGLE, 0, true));
	//submitDrawable(Drawable(++freeDrawableID, Vec2(0, 0), 0, Vec2(10, 10), Vec4(0.5, 0.45, 0.5, 1), Form::RECTANGLE, 0, false));
	//attachTexture(freeDrawableID, "default");
	submitDrawable(Drawable(++freeDrawableID, Vec2(0, 0), 0.0f, Vec2(100000, 10000), Vec4(0, 0, 0, 1), Form::RECTANGLE, 0));

	//take input
	if (keyPressed(KEY::LEFT_ALT) && keyPressed(KEY::F4)) {
		quit();
	}
	if (keyPressed(KEY::UP)) {
		camera.position -= rotate(Vec2(0.0f, -5.0f), camera.rotation) * deltaTime;
	}
	if (keyPressed(KEY::LEFT)) {
		camera.position -= rotate(Vec2(5.0f, 0.0f), camera.rotation) * deltaTime;
	}
	if (keyPressed(KEY::DOWN)) {
		camera.position -= rotate(Vec2(0.0f, 5.0f), camera.rotation) * deltaTime;
	}
	if (keyPressed(KEY::RIGHT)) {
		camera.position -= rotate(Vec2(-5.0f, 0.0f), camera.rotation) * deltaTime;
	}
	if (keyPressed(KEY::NP_ADD)) {
		camera.zoom *= 1.0f + (1.0f * deltaTime);
	}
	if (keyPressed(KEY::NP_SUBTRACT)) {
		camera.zoom *= 1.0f - (1.0f * deltaTime);
	}
	if (keyPressed(KEY::NP_7)) {
		camera.rotation -= 100.0f * deltaTime;
	}
	if (keyPressed(KEY::NP_9)) {
		camera.rotation += 100.0f * deltaTime;
	}
	if (keyPressed(KEY::NP_0)) {
		camera.rotation = 0.0f;
		camera.position = { 0,0 };
		camera.zoom = 1 / 5.0f;
	}

	cursorManipFunc();
	
	//execute scripts
	playerScript.execute(deltaTime);
	healthScript.execute(deltaTime);
	ageScript.execute(deltaTime);
	bulletScript.execute(deltaTime);
	particleScript.execute(deltaTime);
	dummyScript.execute(deltaTime);


	//display performance statistics
	//std::cout << getPerfInfo(5) << '\n';
	//std::cout << "fragmentation: " << world.fragmentation() << std::endl;
	//std::cout << "ent count: " << world.entityCount() << std::endl;
	//std::cout << "ent memsize: " << world.memorySize() << std::endl << std::endl;
	for (auto player : world.viewIDX<Player>()) {
		auto cmps = world.viewComps(player);
		//std::cout << "player speed: " << length(cmps.get<Movement>().velocity) << std::endl;
	}
}

void Game::cursorManipFunc()
{
	auto cursor = world.getIndex(cursorID);
	auto& baseCursor = world.getComp<Base>(cursor);
	auto& colliderCursor = world.getComp<Collider>(cursor);
	baseCursor.position = getPosWorldSpace(getCursorPos());

	baseCursor.rotation = camera.rotation;
	colliderCursor.size = Vec2(1, 1) / camera.zoom / 100.0f;

	for (auto ent : world.viewIDX<Player>()) {
		camera.position = world.getComp<Base>(ent).position;
	}

	//world.getComp<Draw>(cursorID).scale = vec2(1, 1) / camera.zoom / 100.0f;
	if (buttonPressed(BUTTON::MB_LEFT)) {
		world.setStaticsChanged();
		if (cursorManipData.locked) {
			
			if (world.exists(cursorManipData.lockedID)) {
				if (world.hasComp<Movement>(cursorManipData.lockedID)) {
					auto& movControlled = world.getComp<Movement>(cursorManipData.lockedID);
					world.getComp<Movement>(cursorManipData.lockedID) = baseCursor.position - cursorManipData.oldCursorPos;
				}
				auto& baseControlled = world.getComp<Base>(cursorManipData.lockedID);
				auto& colliderControlled = world.getComp<Collider>(cursorManipData.lockedID);
				if (keyPressed(KEY::LEFT_SHIFT)) {	//rotate
					float cursorOldRot = getRotation(normalize(cursorManipData.oldCursorPos - baseControlled.position));
					float cursorNewRot = getRotation(normalize(baseCursor.position - baseControlled.position));
					float diff = cursorNewRot - cursorOldRot;
					baseControlled.rotation += diff;
					cursorManipData.lockedIDDist = baseControlled.position - baseCursor.position;
				}
				else if (keyPressed(KEY::LEFT_CONTROL)) {	//scale
					Vec2 ControlledEntRelativeCoordVec = rotate(Vec2(1, 0), baseControlled.rotation);
					Vec2 cursormovement = baseCursor.position - cursorManipData.oldCursorPos;
					float relativeXMovement = dot(cursormovement, ControlledEntRelativeCoordVec);
					if (dot(-cursorManipData.lockedIDDist, ControlledEntRelativeCoordVec) < 0) {
						relativeXMovement *= -1;
					}
					float relativeYMovement = dot(cursormovement, rotate(ControlledEntRelativeCoordVec, 90));
					if (dot(-cursorManipData.lockedIDDist, rotate(ControlledEntRelativeCoordVec, 90)) < 0) {
						relativeYMovement *= -1;
					}
					colliderControlled.size = colliderControlled.size + Vec2(relativeXMovement, relativeYMovement) * 2;
					world.getComp<Draw>(cursorManipData.lockedID).scale += Vec2(relativeXMovement, relativeYMovement) * 2;
					cursorManipData.lockedIDDist = baseControlled.position - baseCursor.position;
				}
				else {	//move
					baseControlled.position = baseCursor.position + cursorManipData.lockedIDDist;
				}
			}
		}
		else {
			auto [begin, end] = getCollisions(cursor);
			auto iterWIthHighestDrawPrio = begin;
			for (auto iter = begin; iter != end; ++iter) {
				if (world.getComp<Draw>(iter->idB).drawingPrio > world.getComp<Draw>(iterWIthHighestDrawPrio->idB).drawingPrio) {	//higher drawprio found
					iterWIthHighestDrawPrio = iter;
				}
			}
			if (begin != end) {
				cursorManipData.lockedID = iterWIthHighestDrawPrio->idB;
				cursorManipData.lockedIDDist = world.getComp<Base>(iterWIthHighestDrawPrio->idB).position - baseCursor.position;
				cursorManipData.locked = true;
			}
		}

		if (keyPressed(KEY::DELETE) || keyPressed(KEY::BACKSPACE)) {
			if (cursorManipData.locked == true) {
				world.destroy(cursorManipData.lockedID);
			}
		}
	}
	else {
		cursorManipData.locked = false;

		// spawns:
		if (keyPressed(KEY::U)) {
			Vec2 scale = Vec2(0.3f, 0.3f);
			Collider trashCollider = Collider(scale, Form::RECTANGLE);
			Draw trashDraw = Draw(Vec4(1.0f, 1.0f, 1.0f, 1), scale, 0.5f, Form::RECTANGLE, true);
			PhysicsBody trashSolidBody(0.9f, 1.0f, calcMomentOfIntertia(1, scale), 10.0f);

			for (int i = 0; i < cursorManipData.ballSpawnLap.getLaps(getDeltaTime()); i++) {
				Vec2 position = baseCursor.position;
				auto trash = world.createIDX();
				world.addComp<Base>(trash, Base(position, RotaVec2(0)));
				world.addComp<Movement>(trash, Movement(rand() % 1000 / 10000.0f - 0.05f, rand() % 1000 / 10000.0f - 0.05f));
				world.addComp<Collider>(trash, trashCollider);
				world.addComp<Draw>(trash, trashDraw);
				world.addComp<PhysicsBody>(trash, trashSolidBody);
				world.addComp<Health>(trash, Health(100));
				world.spawn(trash);

				auto trashAss = world.createIDX();
				auto cmps = world.viewComps(trashAss);
				cmps.add<Base>();
				cmps.add<Movement>();
				auto coll = trashCollider;
				coll.form = Form::CIRCLE;
				cmps.add<Coll>(coll);
				cmps.add<PhysicsBody>();
				auto draw = trashDraw;
				draw.form = Form::CIRCLE;
				cmps.add<Draw>(draw);
				world.link(trashAss, trash, Vec2(0, 0.15f), 0);
				world.spawn(trashAss);

				trashAss = world.createIDX();
				auto cmps2 = world.viewComps(trashAss);
				cmps2.add<Base>();
				cmps2.add<Movement>();
				coll = trashCollider;
				coll.form = Form::CIRCLE;
				cmps2.add<Coll>(coll);
				cmps2.add<PhysicsBody>();
				draw = trashDraw;
				draw.form = Form::CIRCLE;
				cmps2.add<Draw>(draw);
				world.link(trashAss, trash, Vec2(0, -0.15f), 0);
				world.spawn(trashAss);
			}
		}

		if (keyPressed(KEY::I)) {
			Vec2 scale = Vec2(0.5f, 0.5f);
			Collider trashCollider(scale, Form::RECTANGLE);
			PhysicsBody trashSolidBody(0.00f, 100000000000000000.f, calcMomentOfIntertia(100000000000000000.f, scale), 100.0f);
			Draw trashDraw = Draw(Vec4(1, 1, 1, 1), scale, 0.5f, Form::RECTANGLE);

			for (int i = 0; i < cursorManipData.wallSpawnLap.getLaps(getDeltaTime()); i++) {
				auto trash = world.createIDX();
				world.addComp<Base>(trash, Base(cursorManipData.oldCursorPos, 0));
				world.addComp<Collider>(trash, trashCollider);
				world.addComp<PhysicsBody>(trash, trashSolidBody);
				world.addComp<Draw>(trash, trashDraw);
				world.addComp<TextureRef>(trash, TextureRef("test.png", Vec2(1.f / 16.f * 3.f, 1.f / 16.f * 15.f), Vec2(1.f / 16.f * 4.f, 1.f / 16.f * 16.f)));
				world.spawn(trash);
			}
		}
	}
	cursorManipData.oldCursorPos = getPosWorldSpace(getCursorPos());
}

