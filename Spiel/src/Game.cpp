#include "Game.h"

Game::Game() : 
	Engine("Test", 1600, 900),
	playerScript{ *this },
	healthScript{ *this },
	ageScript   { *this },
	bulletScript{ *this }
{
	auto size = getWindowSize();
	camera.frustumBend = (vec2(1 / getWindowAspectRatio(), 1));

	vec2 cursorScale = { 0.2,0.2 };

	Collider cursorCollider(cursorScale, Form::CIRCLE, true);
	Draw cursorDraw(vec4(1, 0, 0, 1), cursorScale, 0.6f, Form::CIRCLE);
	auto cursor = world.createEnt();
	world.addComp<Base>(cursor);
	world.addComp<Collider>(cursor, cursorCollider);
	world.addComp<Draw>(cursor, cursorDraw);

	cursorID = cursor;
}

void Game::create() {
	camera.zoom = 1 / 5.0f;
	world.loadMap("shit");
}

void Game::update(World& world, float deltaTime) {
	submitDrawableWindowSpace(Drawable(0, vec2(0, 0), 0, vec2(2,2), vec4(0.1, 0.1, 0.1, 1), Form::RECTANGLE, 0));
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
	playerScript.executeAll( deltaTime);
	healthScript.executeAll( deltaTime);
	ageScript.executeAll(    deltaTime);
	bulletScript.executeAll( deltaTime);

	for (auto ent : world.view<Collider, SolidBody>()) {
		if (!world.hasComp<Movement>(ent)) {
		}
	}

	for (auto enemy : world.view<Enemy, Base, Movement>()) {
		assert(world.doesEntExist(enemy));
		auto& enemyComp =	world.getComp<Enemy>(enemy);
		auto& targetPos =	world.getComp<Base>(enemyComp.target).position;
		auto& enemyPos =	world.getComp<Base>(enemy).position;
		auto& targetSpeed = world.getComp<Movement>(enemyComp.target).velocity;
		auto& enemySpeed =	world.getComp<Movement>(enemy).velocity;

		auto staticGrid = getStaticGrid();
		int targetGridX = ceilf((targetPos - staticGrid.minPos).x / staticGrid.cellSize.x);
		int targetGridY = ceilf((targetPos - staticGrid.minPos).y / staticGrid.cellSize.y);
		int enemyGridX = ceilf((enemyPos - staticGrid.minPos).x / staticGrid.cellSize.x);
		int enemyGridY = ceilf((enemyPos - staticGrid.minPos).y / staticGrid.cellSize.y);
		auto d = Drawable(0, vec2(targetGridX * staticGrid.cellSize.x + staticGrid.minPos.x, targetGridY * staticGrid.cellSize.y + staticGrid.minPos.y), 0.7f, staticGrid.cellSize, vec4(1, 0, 0, 1), Form::RECTANGLE, 0);
		submitDrawableWorldSpace(d);

		// guesses distance in grid uniform dist
		auto h = [&](std::pair<int, int> node) -> float {
			float x = node.first - targetGridX;
			float y = node.second - targetGridY;
			return sqrtf(x * x + y * y);
		};

		struct Node {
			Node(float dist_ = 0.0f, int x_ = -1, int y_ = -1) : dist{ dist_ }, x{ x_ }, y{ y_ } {}
			float dist;
			int x;
			int y;
		};

		// float: distance, pair<int,int>: last node (-1,-1) => no last node
		Grid<Node> nodeGrid;
		nodeGrid.resize(staticGrid.getSizeX(), staticGrid.getSizeY(), { 0.0f, -1,-1 });
		std::vector<std::pair<int, int>> frontier;
		frontier.push_back({ enemyGridX, enemyGridY });
		bool foundWay{ false };
		int iteration{ 0 };
		int const maxIteration{ 0 };
		while (!foundWay && iteration < maxIteration && !frontier.empty()) {
			// expand best node in frontier
			std::pair<int, int> bestNode = frontier.front();
			for (auto iter = std::next(frontier.begin()); iter != frontier.end(); ++iter) {
				if (nodeGrid.at(iter->first, iter->second).dist < nodeGrid.at(bestNode.first, bestNode.second).dist) {
					bestNode = *iter;
				}
			}

			// get best expansion node
			Node bestExpansionNode;

		}
	}

	//display performance statistics
	//std::cout << getPerfInfo(5) << '\n';
}

void Game::cursorManipFunc()
{
	auto& baseCursor = world.getComp<Base>(cursorID);
	auto& colliderCursor = world.getComp<Collider>(cursorID);
	baseCursor.position = getPosWorldSpace(getCursorPos());

	baseCursor.rotation = camera.rotation;
	colliderCursor.size = vec2(1, 1) / camera.zoom / 100.0f;

	world.getComp<Draw>(cursorID).scale = vec2(1, 1) / camera.zoom / 100.0f;
	if (buttonPressed(BUTTON::MB_LEFT)) {
		staticsChanged();
		if (cursorManipData.locked) {
			
			if (world.doesEntExist(cursorManipData.lockedID)) {
				auto& movControlled = world.getComp<Movement>(cursorManipData.lockedID);
				auto& baseControlled = world.getComp<Base>(cursorManipData.lockedID);
				auto& colliderControlled = world.getComp<Collider>(cursorManipData.lockedID);
				world.getComp<Movement>(cursorManipData.lockedID) = baseCursor.position - cursorManipData.oldCursorPos;
				if (keyPressed(KEY::LEFT_SHIFT)) {	//rotate
					float cursorOldRot = getAngle(normalize(cursorManipData.oldCursorPos - baseControlled.position));
					float cursorNewRot = getAngle(normalize(baseCursor.position - baseControlled.position));
					float diff = cursorNewRot - cursorOldRot;
					baseControlled.rotation += diff;
					cursorManipData.lockedIDDist = baseControlled.position - baseCursor.position;
				}
				else if (keyPressed(KEY::LEFT_CONTROL)) {	//scale
					vec2 ControlledEntRelativeCoordVec = rotate(vec2(1, 0), baseControlled.rotation);
					vec2 cursormovement = baseCursor.position - cursorManipData.oldCursorPos;
					float relativeXMovement = dot(cursormovement, ControlledEntRelativeCoordVec);
					if (dot(-cursorManipData.lockedIDDist, ControlledEntRelativeCoordVec) < 0) {
						relativeXMovement *= -1;
					}
					float relativeYMovement = dot(cursormovement, rotate(ControlledEntRelativeCoordVec, 90));
					if (dot(-cursorManipData.lockedIDDist, rotate(ControlledEntRelativeCoordVec, 90)) < 0) {
						relativeYMovement *= -1;
					}
					colliderControlled.size = colliderControlled.size + vec2(relativeXMovement, relativeYMovement) * 2;
					world.getComp<Draw>(cursorManipData.lockedID).scale += vec2(relativeXMovement, relativeYMovement) * 2;
					cursorManipData.lockedIDDist = baseControlled.position - baseCursor.position;
				}
				else {	//move
					baseControlled.position = baseCursor.position + cursorManipData.lockedIDDist;
				}
			}
		}
		else {
			auto [begin, end] = getCollisionInfos(cursorID);
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
				world.despawn(cursorManipData.lockedID);
			}
		}
	}
	else {
		cursorManipData.locked = false;

		// spawns:
		if (keyPressed(KEY::U)) {
			vec2 scale = vec2(0.08f, 0.08f);
			Collider trashCollider = Collider(scale, Form::RECTANGLE, true);
			Draw trashDraw = Draw(vec4(1, 1, 1, 1), scale, 0.5f, Form::RECTANGLE);
			SolidBody trashSolidBody(0.2f, 2.5f, 0.1f);

			for (int i = 0; i < cursorManipData.ballSpawnLap.getLaps(getDeltaTime()); i++) {
				auto solid = SolidBody(0.2f, 2.5f, 0.1f);
				auto trash = world.createEnt();
				world.addComp<Base>(trash, Base(baseCursor.position, 0));
				world.addComp<Movement>(trash);
				world.addComp<SolidBody>(trash, trashSolidBody);
				world.addComp<Collider>(trash, trashCollider);
				world.addComp<Draw>(trash, trashDraw);
				world.addComp<Health>(world.getLastEntID(), Health(100));
			}
		}

		if (keyPressed(KEY::I)) {
			vec2 scale = vec2(0.5f, 0.5f);
			Collider trashCollider(scale, Form::RECTANGLE, false);
			SolidBody trashSolidBody(0.00f, 100000000000000000.f, calcMomentOfIntertia(100000000000000000.f, scale));
			Draw trashDraw = Draw(vec4(0, 0, 0, 1), scale, 0.5f, Form::RECTANGLE);

			for (int i = 0; i < cursorManipData.wallSpawnLap.getLaps(getDeltaTime()); i++) {
				auto trash = world.createEnt();
				world.addComp<Base>(trash, Base(cursorManipData.oldCursorPos, 0));
				world.addComp<Collider>(trash, trashCollider);
				world.addComp<SolidBody>(trash, trashSolidBody);
				world.addComp<Draw>(trash, trashDraw);
			}
			staticsChanged();
		}
	}
	cursorManipData.oldCursorPos = getPosWorldSpace(getCursorPos());
}

