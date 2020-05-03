#include "Game.h"

Game::Game() : 
	Engine(world,"Test", 1600, 900),
	playerScript{ *this },
	healthScript{ *this },
	ageScript   { *this },
	bulletScript{ *this },
	particleScript{ *this }
{
	auto size = getWindowSize();
	camera.frustumBend = (Vec2(1 / getWindowAspectRatio(), 1.0f));

	auto cursor = world.createEnt();
	Vec2 cursorScale = { 0.1f,0.1f };
	world.addComp<Base>(cursor);
	Collider cursorCollider(cursorScale, Form::RECTANGLE, true);
	world.addComp<Collider>(cursor, cursorCollider);
	//Draw cursorDraw(vec4(1, 0, 0, 1), cursorScale, 0.6f, Form::CIRCLE);
	//world.addComp<Draw>(cursor, cursorDraw);
	world.spawn(cursor);
	cursorID = cursor;
}

void Game::create() {
	camera.zoom = 1 / 2.5f;
	world.loadMap("shit");

	{
		float angle = 33.19f;
		RotaVec2 rotaVec(angle);
		Vec2 baseVec = rotate({ 1, 0 }, 42.7);
		baseVec = rotate(rotate(baseVec, rotaVec), -42.7);
		std::cout << "angle: " << angle << " translated angle: " << getAngle(baseVec) << std::endl;
	}
	
	//std::cout << "sizeof(std::vector<bool>(200): " << sizeof(std::vector<bool>(100)) * 8 - sizeof(std::vector<bool>(0)) * 8 << std::endl;
	
}

void Game::update(World& world, float deltaTime) {
	submitDrawable(Drawable(++freeDrawableID, Vec2(0, 0), 0, Vec2(2,2), Vec4(1, 1, 1, 1), Form::RECTANGLE, 0, true));
	submitDrawable(Drawable(++freeDrawableID, Vec2(0, 0), 0, Vec2(10, 10), Vec4(0.5, 0.45, 0.5, 1), Form::RECTANGLE, 0, false));
	//attachTexture(freeDrawableID, "nothing");

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
	playerScript.execute( deltaTime);
	healthScript.execute( deltaTime);
	ageScript.execute(    deltaTime);
	bulletScript.execute( deltaTime);
	particleScript.execute(deltaTime);


	//display performance statistics
	//std::cout << getPerfInfo(5) << '\n';
}

void Game::cursorManipFunc()
{
	auto& baseCursor = world.getComp<Base>(cursorID);
	auto& colliderCursor = world.getComp<Collider>(cursorID);
	baseCursor.position = getPosWorldSpace(getCursorPos());

	baseCursor.rotation = camera.rotation;
	colliderCursor.size = Vec2(1, 1) / camera.zoom / 100.0f;

	for (auto ent : world.view<Player>()) {
		camera.position = world.getComp<Base>(ent).position;
	}

	//world.getComp<Draw>(cursorID).scale = vec2(1, 1) / camera.zoom / 100.0f;
	if (buttonPressed(BUTTON::MB_LEFT)) {
		world.staticsChanged();
		if (cursorManipData.locked) {
			
			if (world.doesEntExist(cursorManipData.lockedID)) {
				if (world.hasComp<Movement>(cursorManipData.lockedID)) {
					auto& movControlled = world.getComp<Movement>(cursorManipData.lockedID);
					world.getComp<Movement>(cursorManipData.lockedID) = baseCursor.position - cursorManipData.oldCursorPos;
				}
				auto& baseControlled = world.getComp<Base>(cursorManipData.lockedID);
				auto& colliderControlled = world.getComp<Collider>(cursorManipData.lockedID);
				if (keyPressed(KEY::LEFT_SHIFT)) {	//rotate
					float cursorOldRot = getAngle(normalize(cursorManipData.oldCursorPos - baseControlled.position));
					float cursorNewRot = getAngle(normalize(baseCursor.position - baseControlled.position));
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
			auto [begin, end] = getCollisions(cursorID);
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
			Vec2 scale = Vec2(0.08f, 0.08f);
			Collider trashCollider = Collider(scale, Form::RECTANGLE);
			Draw trashDraw = Draw(Vec4(1, 1, 1, 1), scale, 0.5f, Form::RECTANGLE);
			PhysicsBody trashSolidBody(0.2f, 2.5f, 0.1f, 10.0f);

			for (int i = 0; i < cursorManipData.ballSpawnLap.getLaps(getDeltaTime()); i++) {
				auto trash = world.createEnt();
				world.addComp<Base>(trash, Base(baseCursor.position, 0));
				world.addComp<Movement>(trash);
				world.addComp<PhysicsBody>(trash, trashSolidBody);
				world.addComp<Collider>(trash, trashCollider);
				world.addComp<Draw>(trash, trashDraw);
				world.addComp<Health>(world.getLastEntID(), Health(100));
				world.addComp<TextureRef>(trash, TextureRef("test.png", Vec2(1.f / 16.f * 3.f, 1.f / 16.f * 15.f), Vec2(1.f / 16.f * 4.f, 1.f / 16.f * 16.f)));
				world.spawn(trash);
			}
		}

		if (keyPressed(KEY::I)) {
			Vec2 scale = Vec2(0.5f, 0.5f);
			Collider trashCollider(scale, Form::RECTANGLE);
			PhysicsBody trashSolidBody(0.00f, 100000000000000000.f, calcMomentOfIntertia(100000000000000000.f, scale), 100.0f);
			Draw trashDraw = Draw(Vec4(1, 1, 1, 1), scale, 0.5f, Form::RECTANGLE);

			for (int i = 0; i < cursorManipData.wallSpawnLap.getLaps(getDeltaTime()); i++) {
				auto trash = world.createEnt();
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

