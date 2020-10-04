#include "Game.hpp"
#include <iomanip>

using namespace std_extra;
using std::cout;
using std::endl;

Game::Game() : 
	Engine(world,"Spiel Fenster", 1600, 900),
	playerScript{ *this },
	healthScript{ *this },
	ageScript   { *this },
	bulletScript{ *this },
	particleScript{ *this },
	dummyScript{ *this },
	suckerScript{ *this },
	testerScript{ *this }
{
}

void Game::create() {
	auto size = getWindowSize();
	camera.frustumBend = (Vec2(1 / getWindowAspectRatio(), 1.0f));
	camera.zoom = 1 / 3.5f;
	world.loadMap("world.wrld");
	{
		UIFrame frame({ 500, 500 }, { 300.5f, 300.5f }, world.texture.getId("border.png"));
		frame.borders.y = 20;
		frame.borders.x = 20;
		UIEntity e = ui.createFrame(std::move(frame));
		ui.setAlias("Test", e);

		UIText textEl = UIText("Gravitation: ", world.texture.getId("_pl_ConsolasAtlas.png"));
		textEl.fontSize = { 20.0f };
		textEl.borderDist = { 0.0f, 0.0f };
		textEl.color = { 0, 0, 0, 1.0f };
		textEl.setUpdateFn(
			[&](UIElement* me) 
			{
				Entity player{ 0 };
				for (auto ent : world.entityView<Player>()) {
					player = ent;
				}
				
				std::stringstream ss;
				ss << "Gravitation:\n";
				ss << world.physics.linearEffectAccel << "\n";
				ss << "Player Speed:\n";
				ss << world.getComp<Movement>(player).velocity.length();
				((UIText*)me)->text = ss.str();
			}
		);
		UIEntity id = ui.createElement<UIText>(textEl);

		ui["Test"].giveChild(&ui.getElement<UIText>(id));
	}
}

void Game::update(float deltaTime) {
	world.defragment(World::DefragMode::LAZY);
	{
		Timer t(perfLog.getInputRef("physicstime"));
		{
			movementSystem.execute(world, deltaTime);
		}
		collisionSystem.execute(world, deltaTime);
		for (auto& d : collisionSystem.getDebugDrawables()) submitDrawable(d);
		physicsSystem2.execute(world, deltaTime, collisionSystem);
		for (auto& d : physicsSystem2.getDebugDrawables()) submitDrawable(d);
	}
	{
		Timer t(perfLog.getInputRef("calcRotaVecTime"));
		{
			baseSystem.execute(world);
		}
	}
	gameplayUpdate(deltaTime);
	world.update();

}

void Game::gameplayUpdate(float deltaTime)
{


	for (auto ent : world.entityView<Base, Movement>()) {
		if (world.getComp<Base>(ent).position.length() > 1000)
			world.destroy(ent);
	}
	Vec2 size(10, 13);
	Vec2 startpos(0, getWindowSize().y-size.y);
	drawString(getPerfInfo(5), "_pl_ColnsolasAtlas.png", startpos, size);

	//take input
	const std::string filename("world.wrld");

	if (io.keyJustPressed(KEY::J)) {
		world.loadMap("standart");
		// renderer.resetTextureCache();
	}

	if (io.keyJustPressed(KEY::K)) {
		world.saveMap(filename);
	}
	
	if (io.keyJustPressed(KEY::L)) {
		world.loadMap(filename);
		// renderer.resetTextureCache();
	}

	if (io.keyPressed(KEY::LEFT_ALT, GLOBAL_FOCUS) && io.keyPressed(KEY::F4, GLOBAL_FOCUS)) {
		quit();
	}
	if (io.keyJustPressed(KEY::G)) {
		world.physics.linearEffectAccel += 8 * deltaTime;
	}
	if (io.keyPressed(KEY::H)) {
		world.physics.linearEffectAccel -= 8 * deltaTime;
	}
	if (io.keyPressed(KEY::UP)) {
		camera.position -= rotate(Vec2(0.0f, -5.0f), camera.rotation) * deltaTime;
	}
	if (io.keyPressed(KEY::LEFT)) {
		camera.position -= rotate(Vec2(5.0f, 0.0f), camera.rotation) * deltaTime;
	}
	if (io.keyPressed(KEY::DOWN)) {
		camera.position -= rotate(Vec2(0.0f, 5.0f), camera.rotation) * deltaTime;
	}
	if (io.keyPressed(KEY::RIGHT)) {
		camera.position -= rotate(Vec2(-5.0f, 0.0f), camera.rotation) * deltaTime;
	}
	if (io.keyPressed(KEY::NP_ADD)) {
		camera.zoom *= 1.0f + (1.0f * deltaTime);
	}
	if (io.keyPressed(KEY::NP_SUBTRACT)) {
		camera.zoom *= 1.0f - (1.0f * deltaTime);
	}
	if (io.keyPressed(KEY::NP_7)) {
		camera.rotation -= 100.0f * deltaTime;
	}
	if (io.keyPressed(KEY::NP_9)) {
		camera.rotation += 100.0f * deltaTime;
	}
	if (io.keyPressed(KEY::NP_0)) {
		camera.rotation = 0.0f;
		camera.position = { 0,0 };
		camera.zoom = 1 / 5.0f;
	}
	if (io.keyJustPressed(KEY::B) && io.keyReleased(KEY::LEFT_SHIFT)) {
		if (ui.doesAliasExist("Test")) {
			ui.destroyFrame("Test");
		}
	}
	if (io.keyJustPressed(KEY::B) && io.keyPressed(KEY::LEFT_SHIFT)) {
		if (!ui.doesAliasExist("Test")) {
			UIFrame frame({ 500, 500 }, { 300.5f, 300.5f }, world.texture.getId("border.png"));
			frame.borders.y = 20;
			frame.borders.x = 20;
			ui.setAlias("Test", ui.createFrame(std::move(frame)));

			UIText textEl = UIText("Gravitation: ", world.texture.getId("ConsolasAtlas.png"));
			textEl.fontSize = { 20.0f };
			textEl.borderDist = { 0.0f, 0.0f };
			textEl.color = { 0, 0, 0, 1.0f };
			textEl.setUpdateFn([&](UIElement* me) {
				Entity player{ 0 };
				for (auto ent : world.entityView<Player>()) {
					player = ent;
				}

				std::stringstream ss;
				ss << "Gravitation:\n";
				ss << world.physics.linearEffectAccel << "\n";
				ss << "Player Speed:\n";
				ss << world.getComp<Movement>(player).velocity.length();
				static_cast<UIText*>(me)->text = ss.str();
				});
			auto index = ui.createElement<UIText>(textEl);
			UIText* ptr = &ui.getElement<UIText>(index);
			ui["Test"].giveChild(ptr);
		}
	}
	if (io.keyPressed(KEY::PERIOD, GLOBAL_FOCUS) && io.keyReleased(KEY::LEFT_SHIFT, GLOBAL_FOCUS)) {
		io.setFocus(2);
	}
	if (io.keyPressed(KEY::PERIOD, GLOBAL_FOCUS) && io.keyPressed(KEY::LEFT_SHIFT, GLOBAL_FOCUS)) {
		io.setFocus(STANDART_FOCUS);
	}

	manualInputStateUpdate();

	//execute scripts
	playerScript.execute(deltaTime);
	healthScript.execute(deltaTime);
	ageScript.execute(deltaTime);
	bulletScript.execute(deltaTime);
	particleScript.execute(deltaTime);
	dummyScript.execute(deltaTime);
	suckerScript.execute(deltaTime);
	testerScript.execute<500>(deltaTime, jobManager);

	cursorManipFunc();

	for (auto ent : world.entityView<SpawnerComp>()) {
		auto base = world.getComp<Base>(ent);
		int laps = spawnerLapTimer.getLaps(deltaTime);
		for (int i = 1; i < laps; i++) {
			float rotation = (float)(rand() % 360);
			auto particle = world.create();
			Vec2 movement = rotate(Vec2(5,0), rotation);
			world.addComp<Base>(particle, Base(base.position));
			auto size = Vec2(0.36, 0.36) * ((rand() % 1000) / 1000.0f);
			float gray = (rand() % 1000 / 1000.0f);
			world.addComp<Draw>(particle, Draw(Vec4(gray, gray, gray, 0.3), size, rand() % 1000 / 1000.0f, Form::Circle));
			world.addComp<Movement>(particle, Movement(movement, rand()%10000/100.0f -50.0f));
			//world.addComp<Collider>(particle, Collider(size, Form::Circle, true));
			//world.addComp<PhysicsBody>(particle, PhysicsBody(1, 0.01, 10, 0));
			world.addComp<Age>(particle, Age(rand()%1000/2000.0f*3));
			world.spawn(particle);
		}
	}

	for (auto ent : world.entityView<Base, Movement>()) {
		auto pos = world.getComp<Base>(ent).position;
		if (pos.length() > 1000)
			world.destroy(ent);
	}


	/* display performance statistics */
	//std::cout << getPerfInfo(5) << '\n';
	//std::cout << "fragmentation: " << world.fragmentation() << std::endl;
	//std::cout << "ent count: " << world.entityCount() << std::endl;
	//std::cout << "ent memsize: " << world.maxEntityIndex() << std::endl;
}

void Game::destroy()
{
	ui.destroyFrame("Test");
	world.saveMap("world.wrld");
}

void Game::cursorManipFunc()
{
	Vec2 worldCoord = getPosWorldSpace(getCursorPos());
	Vec2 worldVel = (cursorData.oldPos - worldCoord) * getDeltaTimeSafe();
	Base b = Base(worldCoord, 0);
	Collider c = Collider({ 0.02,0.02 }, Form::Circle);
	renderer.submit(Drawable(0, worldCoord, 1, Vec2(0.02, 0.02), Vec4(1, 0, 0, 1), Form::Circle, RotaVec2(0), DrawMode::WorldSpace));
	if (!cursorData.locked && buttonPressed(BUTTON::MB_LEFT)) {
		std::vector<CollisionInfo> collisions;
		collisionSystem.checkForCollisions(collisions, Collider::DYNAMIC | Collider::SENSOR | Collider::STATIC | Collider::PARTICLE, b, c);
		if (!collisions.empty()) {
			Entity topEntity = collisions.front().indexB;
			EntityId id = world.getId(topEntity);
			cursorData.relativePos = world.getComp<Base>(topEntity).position - worldCoord;
			cursorData.lockedID = id;
			cursorData.locked = true;
		}
	}
	else if (buttonPressed(BUTTON::MB_LEFT)) {
		if (world.isIdValid(cursorData.lockedID)) {
			world.getComp<Base>(cursorData.lockedID).position = cursorData.relativePos + worldCoord;
			if (world.hasComp<Movement>(cursorData.lockedID)) {
				world.getComp<Movement>(cursorData.lockedID).velocity = worldVel;
				world.getComp<Movement>(cursorData.lockedID).angleVelocity = 0;
			}
		}
		else {
			cursorData.locked = false;
		}
	}
	else {
		cursorData.locked = false;
	}

	cursorData.oldPos = worldCoord;


	//Entity cursor = world.getIndex(cursorID);
	//auto& baseCursor = world.getComp<Base>(cursor);
	//auto& colliderCursor = world.getComp<Collider>(cursor);
	//baseCursor.position = getPosWorldSpace(getCursorPos());
	//
	//baseCursor.rotation = camera.rotation;
	//colliderCursor.size = Vec2(1, 1) / camera.zoom / 100.0f;
	//
	//for (auto ent : world.entity_view<Player>()) {
	//	camera.position = world.getComp<Base>(ent).position;
	//}
	//
	////world.getComp<Draw>(cursorID).scale = vec2(1, 1) / camera.zoom / 100.0f;
	//if (buttonPressed(BUTTON::MB_LEFT)) {
	//	world.setStaticsChanged();
	//	if (cursorManipData.locked) {
	//		
	//		if (world.exists(cursorManipData.lockedID)) {
	//			if (world.hasComp<Movement>(cursorManipData.lockedID)) {
	//				auto& movControlled = world.getComp<Movement>(cursorManipData.lockedID);
	//				world.getComp<Movement>(cursorManipData.lockedID) = baseCursor.position - cursorManipData.oldCursorPos;
	//			}
	//			auto& baseControlled = world.getComp<Base>(cursorManipData.lockedID);
	//			auto& colliderControlled = world.getComp<Collider>(cursorManipData.lockedID);
	//			if (io.keyPressed(KEY::LEFT_SHIFT)) {	//rotate
	//				float cursorOldRot = getRotation(normalize(cursorManipData.oldCursorPos - baseControlled.position));
	//				float cursorNewRot = getRotation(normalize(baseCursor.position - baseControlled.position));
	//				float diff = cursorNewRot - cursorOldRot;
	//				baseControlled.rotation += diff;
	//				cursorManipData.lockedIDDist = baseControlled.position - baseCursor.position;
	//			}
	//			else if (io.keyPressed(KEY::LEFT_CONTROL)) {	//scale
	//				Vec2 ControlledEntRelativeCoordVec = rotate(Vec2(1, 0), baseControlled.rotation);
	//				Vec2 cursormovement = baseCursor.position - cursorManipData.oldCursorPos;
	//				float relativeXMovement = dot(cursormovement, ControlledEntRelativeCoordVec);
	//				if (dot(-cursorManipData.lockedIDDist, ControlledEntRelativeCoordVec) < 0) {
	//					relativeXMovement *= -1;
	//				}
	//				float relativeYMovement = dot(cursormovement, rotate(ControlledEntRelativeCoordVec, 90));
	//				if (dot(-cursorManipData.lockedIDDist, rotate(ControlledEntRelativeCoordVec, 90)) < 0) {
	//					relativeYMovement *= -1;
	//				}
	//				colliderControlled.size = colliderControlled.size + Vec2(relativeXMovement, relativeYMovement) * 2;
	//				world.getComp<Draw>(cursorManipData.lockedID).scale += Vec2(relativeXMovement, relativeYMovement) * 2;
	//				cursorManipData.lockedIDDist = baseControlled.position - baseCursor.position;
	//			}
	//			else {	//move
	//				baseControlled.position = baseCursor.position + cursorManipData.lockedIDDist;
	//			}
	//		}
	//	}
	//	else {
	//		std::optional<CollisionInfo> highestPrioColl;
	//		bool first = true;
	//		for (auto collision : collisionSystem.collisions_view(cursor)) {
	//			if (first) {
	//				highestPrioColl = collision;
	//			}
	//			first = false;
	//			if (world.getComp<Draw>(collision.indexB).drawingPrio > world.getComp<Draw>(highestPrioColl.value().indexB).drawingPrio) {	//higher drawprio found
	//				highestPrioColl = collision;
	//			}
	//		}
	//		if (highestPrioColl.has_value()) {
	//			cursorManipData.lockedID = highestPrioColl.value().indexB;
	//			cursorManipData.lockedIDDist = world.getComp<Base>(highestPrioColl.value().indexB).position - baseCursor.position;
	//			cursorManipData.locked = true;
	//		}
	//	}
	//
	//	if (io.keyPressed(KEY::DELETE) || io.keyPressed(KEY::BACKSPACE)) {
	//		if (cursorManipData.locked == true) {
	//			world.destroy(cursorManipData.lockedID);
	//		}
	//	}
	//}
	//else {
	//	cursorManipData.locked = false;
	//
	//	// spawns:
	//	if (io.keyPressed(KEY::U)) {
	//		Vec2 scale = Vec2(0.3f, 0.3f);
	//		Collider trashCollider = Collider(scale, Form::Circle);
	//		PhysicsBody trashSolidBody(0.0f, 1.0f, calcMomentOfIntertia(1, scale), 1.0f);
	//
	//		Vec2 position = baseCursor.position;
	//		// AFTER THIS LINE ALL REFERENCES TO COMPONENTS ARE GETTING INVALIDATED
	//		for (int i = 0; i < cursorManipData.ballSpawnLap.getLaps(getDeltaTime()); i++) {
	//			Vec4 color = Vec4(rand() % 1000 / 1000.0f, rand() % 1000 / 1000.0f, rand() % 1000 / 1000.0f, 1);
	//			Draw trashDraw = Draw(color, scale, 0.5f, Form::Circle);
	//			auto trash = world.index_create();
	//			world.addComp<Base>(trash, Base(position, RotaVec2(0)));
	//			world.addComp<Movement>(trash, Movement());
	//			world.addComp<Collider>(trash, trashCollider);
	//			world.addComp<Draw>(trash, trashDraw);
	//			world.addComp<PhysicsBody>(trash, trashSolidBody);
	//			world.addComp<Health>(trash, Health(100));
	//			world.spawn(trash);
	//		}
	//	}
	//
	//	if (io.keyPressed(KEY::I)) {
	//		Vec2 scale = Vec2(0.5f, 0.5f);
	//		Collider trashCollider(scale, Form::Rectangle);
	//		PhysicsBody trashSolidBody(0.00f, 100000000000000000.f, calcMomentOfIntertia(100000000000000000.f, scale), 1.0f);
	//		Draw trashDraw = Draw(Vec4(1, 1, 1, 1), scale, 0.5f, Form::Rectangle);
	//
	//		for (int i = 0; i < cursorManipData.wallSpawnLap.getLaps(getDeltaTime()); i++) {
	//			auto trash = world.index_create();
	//			world.addComp<Base>(trash, Base(cursorManipData.oldCursorPos, 0));
	//			world.addComp<Collider>(trash, trashCollider);
	//			world.addComp<PhysicsBody>(trash, trashSolidBody);
	//			world.addComp<Draw>(trash, trashDraw);
	//			world.addComp<TextureRef>(trash, TextureRef(world.texture.getId("test.png"), Vec2(1.f / 16.f * 3.f, 1.f / 16.f * 15.f), Vec2(1.f / 16.f * 4.f, 1.f / 16.f * 16.f)));
	//			world.spawn(trash);
	//		}
	//	}
	//}
	//cursorManipData.oldCursorPos = getPosWorldSpace(getCursorPos());
}

