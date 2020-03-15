#include "Game.h"

Game::Game() : 
	Engine("Test", 1600, 900), 
	playerScript{ world.playerCompCtrl, *this },
	healthScript{ world.healthCompCtrl, *this },
	ageScript   { world.ageCompCtrl,    *this },
	bulletScript{ world.bulletCompCtrl, *this },
	triggerScript{ world.triggerCompCtrl, *this },
	ownerScript{ world.ownerCompCtrl, *this },
	slaveScript{ world.slaveCompCtrl, *this }
{
	auto size = getWindowSize();
	camera.frustumBend = (vec2(1 / getWindowAspectRatio(), 1));

	vec2 scaleMouse = { 0.2,0.2 };
	auto cCursor = Entity(vec2(0, 0), 0, Collidable(scaleMouse, Form::RECTANGLE, false, true));
	auto dCursor = CompDataDrawable(vec4(1, 0, 0, 1), scaleMouse, 1.0f, Form::RECTANGLE);
	world.spawnEntity(cCursor, dCursor);
	cursorID = world.getLastID();
}

void Game::create() {
	camera.zoom = 1 / 5.0f;

	vec2 scaleEnt = { 0.4f, 0.8f };
	auto cEnt = Entity(vec2(0, 0), 0.0f, Collidable(scaleEnt, Form::RECTANGLE, true, true,  0.5f, 50.0f, vec2(3,3)));
	auto cDraw = CompDataDrawable(vec4(1, 1, 1, 1), scaleEnt, 0.6f, Form::RECTANGLE, true);
	cEnt.rotation = 45.0f;
	world.spawnEntity(cEnt, cDraw);
	world.playerCompCtrl.registerEntity(world.getLastID(), CompDataPlayer());
	std::cout << "v1 after spawn: " << world.getEntityPtr(1)->velocity << std::endl;

	vec2 scalePortal = { 28, 28 };
	Entity portalC = Entity(vec2(-4, -4), 0, Collidable(scalePortal, Form::CIRCLE, false, true));
	CompDataDrawable portalD = CompDataDrawable(vec4(1, 0, 0, 0.5f), vec2(3,3), 0.49f, Form::CIRCLE);
	world.spawnEntity(portalC, portalD);
	attractorID = world.getLastID();

	portalC.hitboxSize = scalePortal / 16;
	portalC.position = vec2(4, 4);
	portalD.color = vec4(0, 0, 1, 0.5f);
	portalD.drawingPrio = 0.48f;
	world.spawnEntity(portalC, portalD);
	pusherID = world.getLastID();

	Entity wallC = Entity(vec2(0,0), 0, Collidable(vec2(0.4f, 10), Form::RECTANGLE, true, false, 0.3f,  1'000'000'000'000'000.0f, vec2(0,0)));
	CompDataDrawable wallD = CompDataDrawable(vec4(1, 1, 1, 1), vec2(0.4f, 10), 0.5f, Form::RECTANGLE, true);
	for (int i = 0; i < 4; i++) {
		float rotation = 90.0f * i;
		wallC.position = rotate(vec2(-5.f, 0), rotation);
		wallC.rotation = rotation;
		world.spawnEntity(wallC, wallD);
	}
	wallC.hitboxSize = vec2(1, 1);
	wallD.scale = vec2(1, 1);
	for (int j = 0; j < 0; j++) {
		if( j == 0 ) wallC.position = vec2(-10, 5);
		if (j == 1) wallC.position = vec2(10, 5);
		if (j == 2) wallC.position = vec2(-10, -5);
		if (j == 3) wallC.position = vec2(10, -5);
		for (int i = 0; i < 10000; i++) {
			wallC.position = rotate(wallC.position, 0.5f * i);
			world.spawnEntity(wallC, wallD);
		}
	}

	Entity loadTrigC = Entity(vec2(4, 0), 0, Collidable(vec2(0.4f, 1), Form::RECTANGLE, false, true));
	CompDataDrawable loadTrigD = CompDataDrawable(vec4(1, 0, 0, 1), vec2(0.4f, 1), 0.49f, Form::RECTANGLE);
	world.spawnEntity(loadTrigC, loadTrigD);
	world.triggerCompCtrl.registerEntity(world.getLastID(), CompDataTrigger(1));

	int num = 5000;

	vec2 scale = vec2(0.05f, 0.05f);
	Entity trashEntC = Entity(vec2(0, 0), 0.0f, Collidable(scale, Form::CIRCLE, true, true, 0.5f, 0.5f, vec2(0,0)));
	CompDataDrawable trashEntD = CompDataDrawable(vec4(1, 1, 1, 1), scale, 0.5f, Form::CIRCLE, true);
	for (int i = 0; i < num; i++) {
		trashEntC.position = { static_cast<float>(rand() % 1000 / 500.0f - 1.0f) * 5, static_cast<float>(rand() % 1000 / 500.0f - 1.0f) * 5 };

		world.spawnEntity(trashEntC, trashEntD);
		world.healthCompCtrl.registerEntity(world.getLastID(), CompDataHealth(100));
	}
}

void Game::update(World& world, float deltaTime) {
	//take input

	if (keyPressed(KEY::LEFT_ALT) && keyPressed(KEY::F4)) {
		quit();
	}
	if (keyPressed(KEY::UP)) {
		camera.position -= rotate(vec2(0.0f, -5.0f), camera.rotation) * deltaTime;
	}
	if (keyPressed(KEY::LEFT)) {
		camera.position -= rotate(vec2(5.0f, 0.0f), camera.rotation) * deltaTime;
	}
	if (keyPressed(KEY::DOWN)) {
		camera.position -= rotate(vec2(0.0f, 5.0f), camera.rotation) * deltaTime;
	}
	if (keyPressed(KEY::RIGHT)) {
		camera.position -= rotate(vec2(-5.0f, 0.0f), camera.rotation) * deltaTime;
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
	playerScript.executeAll(world, deltaTime);
	healthScript.executeAll(world, deltaTime);
	ageScript.executeAll(world, deltaTime);
	bulletScript.executeAll(world, deltaTime);
	triggerScript.executeAll(world, deltaTime);
	ownerScript.executeAll(world, deltaTime);
	slaveScript.executeAll(world, deltaTime);

	//display performance statistics
	//std::cout << getPerfInfo(5) << '\n';
	
	auto attractor = world.getEntityPtr(attractorID);
	auto pusher = world.getEntityPtr(pusherID);
	float acceleration = 0.1f;
	float minDist = -attractor->getRadius() + 0.2f;

	auto [begin, end] = getCollisionInfos(attractorID);
	if (begin != end) {
		for (auto iter = begin; iter != end; iter++) {
			auto otherPtr = world.getEntityPtr(iter->idB);
			if (otherPtr->isSolid() && otherPtr->isDynamic()) {
				otherPtr->acceleration += normalize(attractor->getPos() - otherPtr->getPos()) * acceleration * powf((iter->clippingDist / attractor->getRadius()), 2);
				if (iter->clippingDist < minDist) {
					vec2 otherToCenter = (attractor->position + pusher->position)/2.0f - otherPtr->getPos();
					otherPtr->position += otherToCenter * 2;
				}
			}
		}
	}
	auto [begin2, end2] = getCollisionInfos(pusherID);
	if (begin2 != end2) {
		for (auto iter = begin2; iter != end2; iter++) {
			auto otherPtr = world.getEntityPtr(iter->idB);
			if (otherPtr->isSolid()) {
				otherPtr->acceleration += normalize(pusher->getPos() - otherPtr->getPos()) * -acceleration * powf((iter->clippingDist / pusher->getRadius()), 2);
			}
		}
	}
	
}

void Game::cursorManipFunc()
{
	auto* cursor = world.getEntityPtr(cursorID);
	cursor->position = getPosWorldSpace(getCursorPos());
	cursor->rotation = camera.rotation;
	cursor->hitboxSize = vec2(1, 1) / camera.zoom / 100.0f;
	world.drawableCompCtrl.getComponent(cursorID)->scale = vec2(1, 1) / camera.zoom / 100.0f;
	if (buttonPressed(BUTTON::MB_LEFT)) {
		if (cursorManipData.locked) {
			auto* controlledEnt = world.getEntityPtr(cursorManipData.lockedID);
			if (controlledEnt != nullptr) {
				controlledEnt->velocity = 0;
				if (keyPressed(KEY::LEFT_SHIFT)) {	//rotate
					float cursorOldRot = getAngle(normalize(cursorManipData.oldCursorPos - controlledEnt->position));
					float cursorNewRot = getAngle(normalize(cursor->position - controlledEnt->position));
					float diff = cursorNewRot - cursorOldRot;
					controlledEnt->rotation += diff;
					cursorManipData.lockedIDDist = controlledEnt->getPos() - cursor->getPos();
				}
				else if (keyPressed(KEY::LEFT_CONTROL)) {	//scale
					vec2 ControlledEntRelativeCoordVec = rotate(vec2(1, 0), controlledEnt->rotation);
					vec2 cursormovement = cursor->position - cursorManipData.oldCursorPos;
					float relativeXMovement = dot(cursormovement, ControlledEntRelativeCoordVec);
					if (dot(-cursorManipData.lockedIDDist, ControlledEntRelativeCoordVec) < 0) {
						relativeXMovement *= -1;
					}
					float relativeYMovement = dot(cursormovement, rotate(ControlledEntRelativeCoordVec, 90));
					if (dot(-cursorManipData.lockedIDDist, rotate(ControlledEntRelativeCoordVec, 90)) < 0) {
						relativeYMovement *= -1;
					}
					controlledEnt->hitboxSize = controlledEnt->hitboxSize + vec2(relativeXMovement, relativeYMovement) * 2;
					world.drawableCompCtrl.getComponent(cursorManipData.lockedID)->scale += vec2(relativeXMovement, relativeYMovement) * 2;
					cursorManipData.lockedIDDist = controlledEnt->getPos() - cursor->getPos();
				}
				else {	//move
					controlledEnt->position = cursor->getPos() + cursorManipData.lockedIDDist;
				}
			}
		}
		else {
			auto [begin, end] = getCollisionInfos(cursorID);
			auto iterWIthHighestDrawPrio = begin;
			for (auto iter = begin; iter != end; ++iter) {
				if (world.drawableCompCtrl.getComponent(iter->idB)->drawingPrio > world.drawableCompCtrl.getComponent(iterWIthHighestDrawPrio->idB)->drawingPrio) {	//higher drawprio found
					iterWIthHighestDrawPrio = iter;
				}
			}
			if (begin != end) {
				cursorManipData.lockedID = iterWIthHighestDrawPrio->idB;
				cursorManipData.lockedIDDist = world.getEntityPtr(iterWIthHighestDrawPrio->idB)->getPos() - cursor->getPos();
				cursorManipData.locked = true;
			}
		}

		if (keyPressed(KEY::DELETE) || keyPressed(KEY::BACKSPACE)) {
			if (cursorManipData.locked == true) {
				world.despawn(cursorManipData.lockedID);
			}
		}
	}
	else {
		cursorManipData.locked = false;

		// spawns:
		if (keyPressed(KEY::U)) {
			vec2 scale = vec2(0.05f, 0.05f);
			Entity trashEntC = Entity(cursor->position, 0.0f, Collidable(scale, Form::CIRCLE, true, true, 0.00f, 0.5f, vec2(0, 0)));
			CompDataDrawable trashEntD = CompDataDrawable(vec4(1, 1, 1, 1), scale, 0.5f, Form::CIRCLE);

			for (int i = 0; i < cursorManipData.ballSpawnLap.getLaps(getDeltaTime()); i++) {
				world.spawnEntity(trashEntC, trashEntD);
				world.healthCompCtrl.registerEntity(world.getLastID(), CompDataHealth(100));
			}
		}

		if (keyPressed(KEY::I)) {
			vec2 scale = vec2(0.5f, 0.5f);
			Entity trashEntC = Entity(cursor->position, 0.0f, Collidable(scale, Form::RECTANGLE, true, false, 0.00f, 100000000900000000.f, vec2(0, 0)));
			CompDataDrawable trashEntD = CompDataDrawable(vec4(0.5, 0.5, 0.5, 1), scale, 0.5f, Form::RECTANGLE);

			for (int i = 0; i < cursorManipData.wallSpawnLap.getLaps(getDeltaTime()); i++) {
				world.spawnEntity(trashEntC, trashEntD);
			}
		}
	}
	cursorManipData.oldCursorPos = cursor->position;
}