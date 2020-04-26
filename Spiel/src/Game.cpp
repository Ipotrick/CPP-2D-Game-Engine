#include "Game.h"

Game::Game() : 
	Engine(world,"Test", 1600, 900),
	playerScript{ *this },
	healthScript{ *this },
	ageScript   { *this },
	bulletScript{ *this }
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

	cursorID = cursor;
}

void Game::create() {
	camera.zoom = 1 / 5.0f;
	world.loadMap("shit");

	{
		float angle = 33.19f;
		RotaVec2 rotaVec(angle);
		Vec2 baseVec = rotate({ 1, 0 }, 42.7);
		baseVec = rotate(rotate(baseVec, rotaVec), -42.7);
		std::cout << "angle: " << angle << " translated angle: " << getAngle(baseVec) << std::endl;
	}
	/*
	{
		float angle = 33.19f;
		vec2 size = vec2(4.4f, 1.42f);
		vec2 boundsSize1 = boundsSize(Form::RECTANGLE, size, angle);
		vec2 rotaVec = rotationVec(angle);
		vec2 boundsSize2 = boundsSizeFast(Form::RECTANGLE, size, rotaVec);
		std::cout << "boundssize1: " << boundsSize1 << " boundssize2: " << boundsSize2 << std::endl;
	}
	*/
}

void Game::update(World& world, float deltaTime) {
	submitDrawable(Drawable(++freeDrawableID, Vec2(0, 0), 0, Vec2(2,2), Vec4(1, 1, 1, 0.5), Form::RECTANGLE, 0, true));

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
	playerScript.executeAll( deltaTime);
	healthScript.executeAll( deltaTime);
	ageScript.executeAll(    deltaTime);
	bulletScript.executeAll( deltaTime);

	/*
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
#ifdef DEBUG_PATHFINDING
		auto d = Drawable(0, Vec2(targetGridX * staticGrid.cellSize.x + staticGrid.minPos.x, targetGridY * staticGrid.cellSize.y + staticGrid.minPos.y), 0.7f, staticGrid.cellSize, Vec4(1, 0, 0, 1), Form::RECTANGLE, RotaVec2(0), false);
		submitDrawable(d);
#endif
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
	}*/

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
			PhysicsBody trashSolidBody(0.2f, 2.5f, 0.1f, 1.0f);

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
			PhysicsBody trashSolidBody(0.00f, 100000000000000000.f, calcMomentOfIntertia(100000000000000000.f, scale), 1.0f);
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

