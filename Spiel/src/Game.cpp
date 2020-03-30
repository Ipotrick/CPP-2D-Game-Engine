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
	world.loadMap("shit");
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
	std::cout << getPerfInfo(5) << '\n';
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

