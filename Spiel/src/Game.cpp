#include "Game.h"

Game::Game() : 
	Engine("Test", 1600, 900), 
	playerScript{ world.getCtrl<Player>(), *this },
	healthScript{ world.getCtrl<Health>(), *this },
	ageScript   { world.getCtrl<Age>(),    *this },
	bulletScript{ world.getCtrl<Bullet>(), *this }
{
	auto size = getWindowSize();
	camera.frustumBend = (vec2(1 / getWindowAspectRatio(), 1));

	vec2 scaleMouse = { 0.2,0.2 };
	auto cCursor = Entity(vec2(0, 0), 0, Collidable(scaleMouse, Form::RECTANGLE, true));
	auto dCursor = Draw(vec4(1, 0, 0, 1), scaleMouse, 1.0f, Form::RECTANGLE);
	world.spawnEntity(cCursor, dCursor);
	cursorID = world.getLastID();
}

void Game::create() {
	
	camera.zoom = 1 / 5.0f;

	vec2 scaleEnt = { 0.4f, 0.8f };
	auto cEnt = Entity(vec2(0, 0), 0.0f, Collidable(scaleEnt, Form::RECTANGLE, true, vec2(0,0)));
	auto cDraw = Draw(vec4(0, 0, 0, 1), scaleEnt, 0.6f, Form::RECTANGLE, true);
	world.spawnSolidEntity(cEnt, cDraw, SolidBody(0.5f, 70.0f));
	world.addComp<Player>(world.getLastID(), Player());
	auto playerID = world.getLastID();
	world.addComp<Composit<4>>(playerID, Composit<4>());
	
	auto slaveC = Entity(vec2(0.5, 0), 0.0f, Collidable(vec2(scaleEnt.x / sqrt(2)), Form::RECTANGLE, true, vec2(3, 0)));
	auto slaveD = Draw(vec4(0, 0, 0, 1), vec2(scaleEnt.x / sqrt(2)), 0.59f, Form::RECTANGLE, true);
	world.spawnSolidSlave(slaveC, slaveD, playerID, vec2(0, -0.4), 45);
	world.spawnSolidSlave(slaveC, slaveD, playerID, vec2(0, 0.4), 45);

	vec2 scalePortal = { 28, 28 };
	Entity portalC = Entity(vec2(-4, -4), 0, Collidable(scalePortal, Form::CIRCLE, true));
	Draw portalD = Draw(vec4(1, 0, 0, 0.5f), vec2(3, 3), 0.3f, Form::CIRCLE);
	world.spawnEntity(portalC, portalD);
	attractorID = world.getLastID(); 

	portalC.size = vec2(3, 3);
	portalC.position = vec2(4, 4);
	portalD.color = vec4(0, 0, 1, 0.5f);
	portalD.drawingPrio = 0.31f;
	world.spawnEntity(portalC, portalD);
	pusherID = world.getLastID();

	Entity wallC = Entity(vec2(0,0), 0, Collidable(vec2(0.4f, 10), Form::RECTANGLE, false, vec2(0,0)));
	Draw wallD = Draw(vec4(0, 0, 0, 1), vec2(0.4f, 10), 0.5f, Form::RECTANGLE, true);
	for (int i = 0; i < 4; i++) {
		float rotation = 90.0f * i;
		wallC.position = rotate(vec2(-5.f, 0), rotation);
		wallC.rotation = rotation;
		world.spawnSolidEntity(wallC, wallD, SolidBody(0.3f, 1'000'000'000'000'000.0f));
	}

	int num = 500;

	vec2 scale = vec2(0.05f, 0.05f);
	Entity trashEntC = Entity(vec2(0, 0), 0.0f, Collidable(scale, Form::CIRCLE, true, vec2(0,0)));
	//trashEntC.particle = true;
	Draw trashEntD = Draw(vec4(0,0, 0, 1), scale, 0.5f, Form::CIRCLE, true);
	auto slaveEntC = trashEntC;
	slaveEntC.form = Form::RECTANGLE;
	auto slaveEntD = trashEntD;
	slaveEntD.color = vec4(0,0,0, 1);
	slaveEntD.drawingPrio += 0.01f;
	slaveEntD.form = Form::RECTANGLE;
	auto trashSolid = SolidBody(0.1f, 0.5f);
	trashSolid.momentOfInertia = 0.005f;
	for (int i = 0; i < num; i++) {

		trashEntC.position = { static_cast<float>(rand() % 1000 / 500.0f - 1.0f) * 4.6f, static_cast<float>(rand() % 1000 / 500.0f - 1.0f) * 4.6f };

		world.spawnSolidEntity(trashEntC, trashEntD, trashSolid);
		world.addComp<Health>(world.getLastID(), Health(100));
		world.addComp<Composit<4>>(world.getLastID(), Composit<4>());
		world.spawnSolidSlave(slaveEntC, slaveEntD, world.getLastID(), vec2(0.025f, 0), 0);
	}
}

void Game::update(World& world, float deltaTime) {
	
	submitDrawableWindowSpace(Drawable(0, vec2(0, 0), 0, vec2(2,2), vec4(0.5, 0.5, 0.5, 1), Form::RECTANGLE, 0));
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
	playerScript.executeAll( world, deltaTime);
	healthScript.executeAll( world, deltaTime);
	ageScript.executeAll(    world, deltaTime);
	bulletScript.executeAll( world, deltaTime);

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
				otherPtr->velocity += normalize(attractor->getPos() - otherPtr->getPos()) * acceleration * powf((iter->clippingDist / attractor->getRadius()), 2) * deltaTime;
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
				otherPtr->velocity += normalize(pusher->getPos() - otherPtr->getPos()) * -acceleration * powf((iter->clippingDist / pusher->getRadius()), 2) * deltaTime;
			}
		}
	}
}

void Game::cursorManipFunc()
{
	auto* cursor = world.getEntityPtr(cursorID);
	cursor->position = getPosWorldSpace(getCursorPos());
	cursor->rotation = camera.rotation;
	cursor->size = vec2(1, 1) / camera.zoom / 100.0f;
	world.getCompPtr<Draw>(cursorID)->scale = vec2(1, 1) / camera.zoom / 100.0f;
	if (buttonPressed(BUTTON::MB_LEFT)) {
		staticsChanged();
		if (cursorManipData.locked) {
			auto* controlledEnt = world.getEntityPtr(cursorManipData.lockedID);
			if (controlledEnt != nullptr) {
				controlledEnt->velocity = cursor->position - cursorManipData.oldCursorPos;
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
					controlledEnt->size = controlledEnt->size + vec2(relativeXMovement, relativeYMovement) * 2;
					world.getCompPtr<Draw>(cursorManipData.lockedID)->scale += vec2(relativeXMovement, relativeYMovement) * 2;
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
				if (world.getCompPtr<Draw>(iter->idB)->drawingPrio > world.getCompPtr<Draw>(iterWIthHighestDrawPrio->idB)->drawingPrio) {	//higher drawprio found
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
			vec2 scale = vec2(0.08f, 0.08f);
			Entity trashEntC = Entity(cursor->position, 0.0f, Collidable(scale, Form::RECTANGLE, true, vec2(0, 0)));
			Draw trashEntD = Draw(vec4(1, 1, 1, 1), scale, 0.5f, Form::RECTANGLE);

			for (int i = 0; i < cursorManipData.ballSpawnLap.getLaps(getDeltaTime()); i++) {
				auto solid = SolidBody(0.2f, 2.5f);
				solid.momentOfInertia = 0.1f;
				world.spawnSolidEntity(trashEntC, trashEntD, solid);
				world.addComp<Health>(world.getLastID(), Health(100));
			}
		}

		if (keyPressed(KEY::I)) {
			vec2 scale = vec2(0.5f, 0.5f);
			Entity trashEntC = Entity(cursor->position, 0.0f, Collidable(scale, Form::RECTANGLE, false, vec2(0, 0)));
			Draw trashEntD = Draw(vec4(0, 0, 0, 1), scale, 0.5f, Form::RECTANGLE);

			for (int i = 0; i < cursorManipData.wallSpawnLap.getLaps(getDeltaTime()); i++) {
				world.spawnSolidEntity(trashEntC, trashEntD, SolidBody(0.00f, 100000000000000000.f));
			}
			staticsChanged();
		}
	}
	cursorManipData.oldCursorPos = getPosWorldSpace(getCursorPos());
}

