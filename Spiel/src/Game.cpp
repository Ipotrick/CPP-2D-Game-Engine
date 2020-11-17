#include "Game.hpp"

#include <iomanip>

#include "log/Log.hpp"
#include "GameComponents.hpp"
#include "serialization/YAMLSerializer.hpp"

#include "transformScript.hpp"
#include "movementScript.hpp"
#include "BasicScripts.hpp"
#include "ParticleScript.hpp"
#include "SuckerScript.hpp"
#include "TesterScript.hpp"
#include "drawScript.hpp"
#include "LayerConstants.hpp"

using namespace util;

Game::Game()
	: Engine("Spiel Fenster", 1600, 900)
{
	renderer.setLayerCount(LAYER_MAX);
	renderer.getLayer(LAYER_WORLD_BACKGROUND).renderMode = RenderSpace::WorldSpace;
	renderer.getLayer(LAYER_WORLD_MIDGROUND).renderMode = RenderSpace::WorldSpace;
	renderer.getLayer(LAYER_WORLD_FOREGROUND).renderMode = RenderSpace::WorldSpace;
	renderer.getLayer(LAYER_FIRST_UI).renderMode = RenderSpace::PixelSpace;
	renderer.getLayer(LAYER_SECOND_UI).renderMode = RenderSpace::PixelSpace;
}

#define UI_CREATE(parent, code, Type, name) \
{ \
	Type name; \
	code \
	parent.addChild(Engine::ui.createAndGet(name)); \
}

#define UI_TEXT(parent, code)		UI_CREATE(parent, code, UIText, me)

#define UI_SEPERATOR(parent, code)	UI_CREATE(parent, code, UISeperator, me)

#define UI_TEXT_UPDATE(code) \
me.setUpdateFn([&](UIElement* e){ UIText& me = *((UIText*)e); code });

void Game::create() {
	auto size = getWindowSize();
	renderer.getCamera().frustumBend = (Vec2(1 / getWindowAspectRatio(), 1.0f));
	renderer.getCamera().zoom = 1 / 3.5f;

	const float firstRowWidth = 90.0f;
	const Vec2 textFieldSize{ firstRowWidth , 17.0f };
	const auto font = renderer.makeSmallTexRef(TextureInfo("_pl_ConsolasLowRes.png"));

	world.loadMap("standart");

	auto makeRenderStatsUI = [&](auto& parent) {
		UICollapsable c("Rendering Statics:", font);
		c.setHeadLength(20);
		{
			UIList8 l;
			l.setPadding({ 5.0f, 0.0f });
			{
				UIPair uicountPair;
				uicountPair.setHorizontal();
				{
					UIText uiCountText("ui count:", font);
					uiCountText.setSize(textFieldSize);
					uicountPair.setFirst(ui.createAndGet(uiCountText));
				}
				{
					UIText uiCountText2("", font, [&](UIElement* e) {
						((UIText*)e)->text = std::to_string(ui.activeElementCount());
						}
					);
					uiCountText2.setSize(textFieldSize);
					uicountPair.setSecond(ui.createAndGet(uiCountText2));
				}
				l.addChild(ui.createAndGet(uicountPair));
			}
			{
				UIPair uidcountPair;
				uidcountPair.setHorizontal();
				{
					UIText uidCountText("ui draw:", font);
					uidCountText.setSize(textFieldSize);
					uidcountPair.setFirst(ui.createAndGet(uidCountText));
				}
				{
					UIText uidCountText2("", font, [&](UIElement* e) { ((UIText*)e)->text = std::to_string(ui.drawCount()); });
					uidCountText2.setSize(textFieldSize);
					uidcountPair.setSecond(ui.createAndGet(uidCountText2));
				}
				l.addChild(ui.createAndGet(uidcountPair));
			}
			{
				UIPair drawCallPair;
				drawCallPair.setHorizontal();
				{
					UIText drawcallText("Drawcalls:", font);
					drawcallText.setSize(textFieldSize);
					drawCallPair.setFirst(ui.createAndGet(drawcallText));
				}
				{
					UIText drawcallText2("", font, [&](UIElement* e) { ((UIText*)e)->text = std::to_string(renderer.getDrawCalls()); });
					drawcallText2.setSize(textFieldSize);
					drawCallPair.setSecond(ui.createAndGet(drawcallText2));
				}
				l.addChild(ui.createAndGet(drawCallPair));
			}

			c.addChild(ui.createAndGet(l));
		}
		parent.addChild(ui.createAndGet(c));
	};

	UIFrame frame;
	frame.layer = LAYER_SECOND_UI;
	frame.setWidth(200);
	frame.setPadding({ 5.0f, 5.0f });
	frame.anchor.setLeftAbsolute(10);
	frame.anchor.setTopAbsolute(10);
	{
		UIList16 list;
		list.setSpacing(5.0f);
		UI_TEXT(list,
			me.text = "Statistics:";
			me.fontTexture = font;
			me.setSize(textFieldSize);
			me.textAnchor.setCenterHorizontal();
			me.anchor.setCenterHorizontal();
		)
		UI_SEPERATOR(list, me.setHorizontal();)
		{
			UIPair entCountPair;
			entCountPair.setHorizontal();
			{
				UIText entitiesText("Entities:", font);
				entitiesText.setSize(textFieldSize);
				entCountPair.setFirst(ui.createAndGet(entitiesText));
			}
			{
				UIText entitiesText2("", font, [&](UIElement* e) { ((UIText*)e)->text = std::to_string(world.size()); });
				entitiesText2.setSize(textFieldSize);
				entCountPair.setSecond(ui.createAndGet(entitiesText2));
			}
			list.addChild(ui.createAndGet(entCountPair));
		}
		makeRenderStatsUI(list);
		{
			UIPair ticksPair;
			ticksPair.setHorizontal();
			{
				UIText ticksText("Ticks/s:", font);
				ticksText.setSize(textFieldSize);
				ticksPair.setFirst(ui.createAndGet(ticksText));
			}
			{
				UIText ticksText2("", font, [&](UIElement* e) { ((UIText*)e)->text = std::to_string(1.0f / getDeltaTime(100)); });
				ticksText2.setSize(textFieldSize);
				ticksPair.setSecond(ui.createAndGet(ticksText2));
			}
			list.addChild(ui.createAndGet(ticksPair));
		}
		frame.addChild(ui.createAndGet(list));
	}
	ui.createFrame(frame, "Statiscics");

}

void Game::update(float deltaTime) {
	if (bLoading) {
		if (jobManager.finished(loadingWorkerTag)) {
			jobManager.clear(loadingWorkerTag);
			bLoading = false;
			in.returnFocus();
			in.returnMouseFocus();
			world = *loadedWorld;
			delete loadedWorld;
			if (ui.exists("loadingtext")) {
				ui.destroyFrame("loadingtext");
			}
			ui.update();
		}
	}
	else {
		renderer.submit(Drawable(0, { 0,0 }, -1.0f, { 2, 2 }, { 0.2, 0.4, 1.0f, 1.0f }, Form::Rectangle, RotaVec2{ 0 }, RenderSpace::WindowSpace));
		{
			collisionSystem.execute(world, deltaTime);
			for (auto& d : collisionSystem.getDebugDrawables()) renderer.submit(d);
			physicsSystem2.execute(world, deltaTime, collisionSystem);
			for (auto& d : physicsSystem2.getDebugDrawables()) renderer.submit(d);

			for (auto [ent, t] : world.entityComponentView<Transform>()) {
				transformScript(ent, t);
				if (world.hasComp<Movement>(ent)) movementScript(ent, t, world.getComp<Movement>(ent), deltaTime);
			}
		}

		in.manualUpdate(renderer.getCamera());

		gameplayUpdate(deltaTime);

		for (auto [ent, t, d] : world.entityComponentView<Transform, Draw>()) drawScript(ent, t, d);

		world.update();
	}
}

void Game::gameplayUpdate(float deltaTime)
{
	if (in.keyPressed(Key::LEFT_ALT, Focus::Global) && in.keyPressed(Key::F4, Focus::Global)) {
		Engine::quit();
	}
	if (in.keyPressed(Key::G)) {
		world.physics.linearEffectAccel += 8 * deltaTime;
	}
	if (in.keyPressed(Key::H)) {
		world.physics.linearEffectAccel -= 8 * deltaTime;
	}
	if (in.keyPressed(Key::UP)) {
		renderer.getCamera().position -= rotate(Vec2(0.0f, -5.0f), renderer.getCamera().rotation) * deltaTime;
	}
	if (in.keyPressed(Key::LEFT)) {
		renderer.getCamera().position -= rotate(Vec2(5.0f, 0.0f), renderer.getCamera().rotation) * deltaTime;
	}
	if (in.keyPressed(Key::DOWN)) {
		renderer.getCamera().position -= rotate(Vec2(0.0f, 5.0f), renderer.getCamera().rotation) * deltaTime;
	}
	if (in.keyPressed(Key::RIGHT)) {
		renderer.getCamera().position -= rotate(Vec2(-5.0f, 0.0f), renderer.getCamera().rotation) * deltaTime;
	}
	if (in.keyPressed(Key::NP_ADD)) {
		//renderer.getCamera().zoom += 2;
		renderer.getCamera().zoom *= 1.0f + (1.0f * deltaTime);
	}
	if (in.keyPressed(Key::NP_SUBTRACT)) {
		renderer.getCamera().zoom *= 1.0f - (1.0f * deltaTime);
	}
	if (in.keyPressed(Key::NP_7)) {
		renderer.getCamera().rotation -= 100.0f * deltaTime;
	}
	if (in.keyPressed(Key::NP_9)) {
		renderer.getCamera().rotation += 100.0f * deltaTime;
	}
	if (in.keyPressed(Key::NP_0)) {
		renderer.getCamera().rotation = 0.0f;
		renderer.getCamera().position = { 0, 0 };
		renderer.getCamera().zoom = 1 / 5.0f;
		uiContext.scale = 1.0f;
	}
	if (in.keyJustPressed(Key::B) && in.keyReleased(Key::LEFT_SHIFT)) {
		if (ui.exists("Statiscics")) {
			ui.getFrame("Statiscics").disable();
		}
	}
	if (in.keyJustPressed(Key::B) && in.keyPressed(Key::LEFT_SHIFT)) {
		if (ui.exists("Statiscics")) {
			ui.getFrame("Statiscics").enable();
		}
	}
	if (in.keyPressed(Key::PERIOD, Focus::Global) && in.keyReleased(Key::LEFT_SHIFT, Focus::Global)) {
		in.takeFocus(Focus::UI);
	}
	if (in.keyPressed(Key::PERIOD, Focus::Global) && in.keyPressed(Key::LEFT_SHIFT, Focus::Global)) {
		in.takeFocus(Focus::Standard);
	}
	if (in.keyPressed(Key::I)) {
		uiContext.scale = clamp(uiContext.scale - deltaTime, 0.1f, 10.0f);
	}
	if (in.keyPressed(Key::O)) {
		uiContext.scale = clamp(uiContext.scale + deltaTime, 0.1f, 10.0f);
	}
	if (in.keyPressed(Key::J)) {
		world = World();
		world.loadMap("standart");
		ui.update();
	}
	if (in.keyJustPressed(Key::K)) {
		World w = world;
		auto job = LambdaJob(
			[w](int id) mutable {
				Monke::log("Start saving...");

				std::ofstream of("dump.yaml");
				if (of.good()) {
					YAMLWorldSerializer s(w);
					of << s.serializeToString();
					of.close();
				}
				Monke::log("Finished saving!");
			}
		);
		job.bEnableWaiting = false;
		job.bSelfDestruct = true;
		jobManager.addJob(new LambdaJob(job));
	}
	if (in.keyJustPressed(Key::L)) {
		bLoading = true;
		in.takeFocus(Focus::UI);
		in.takeMouseFocus(Focus::UI);

		auto* job =  new LambdaJob(
			[&](int id) 
			{
				Monke::log("Start loading...");
				loadedWorld = new World();
				std::ifstream ifstream("dump.yaml");
				if (ifstream.good()) {
					YAMLWorldSerializer s(*loadedWorld);
					std::string str;
					std::getline(ifstream, str, '\0');
					s.deserializeString(str);
				}
				Monke::log("Finished loading!");
			}
		);
		job->bSelfDestruct = true;

		loadingWorkerTag = jobManager.addJob(job);

		UIFrame f;
		f.setSize({ 1000, 200 });
		f.setBorders(5);
		f.anchor.setCenterHorizontal();
		f.anchor.setCenterVertical();

		UIText text("Loading..", renderer.makeSmallTexRef(TextureInfo("ConsolasAtlas.png")));
		text.setSize({ 1000, 200 });
		text.textAnchor.setCenterHorizontal();
		text.textAnchor.setCenterVertical();
		text.fontSize = { 30, 100 };

		f.addChild(ui.createAndGet(text));

		ui.createFrame(f, "loadingtext");
	}

	//execute scripts
	{
		LogTimer t(std::cout, "time taken for scripts: ");
		for (auto [ent, comp] : world.entityComponentView<Health>()) healthScript(ent, comp, deltaTime);
		for (auto [ent, comp] : world.entityComponentView<Player>()) playerScript(ent, comp, deltaTime);
		for (auto [ent, comp] : world.entityComponentView<Age>()) ageScript(ent, comp, deltaTime);
		for (auto [ent, comp] : world.entityComponentView<Bullet>()) bulletScript(ent, comp, deltaTime);
		for (auto [ent, comp] : world.entityComponentView<ParticleScriptComp>()) particleScript(ent, comp, deltaTime);
		for (auto [ent, comp] : world.entityComponentView<SuckerComp>()) suckerScript(ent, comp, deltaTime);
		for (auto [ent, comp] : world.entityComponentView<Tester>()) testerScript(ent, comp, deltaTime);
	}

	cursorManipFunc();

	for (auto ent : world.entityView<SpawnerComp>()) {
		const Transform base = world.getComp<Transform>(ent);
		int laps = spawnerLapTimer.getLaps(deltaTime);
		for (int i = 1; i < laps; i++) {
			float rotation = (float)(rand() % 360);
			auto particle = world.create();
			Vec2 movement = rotate(Vec2(5,0), rotation);
			world.addComp<Transform>(particle, Transform(base.position));
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

	for (auto ent : world.entityView<Movement, Transform>()) {
		auto pos = world.getComp<Transform>(ent).position;
		if (pos.length() > 1000)
			world.destroy(ent);
	}
}

void Game::destroy()
{
	ui.destroyFrame("Statiscics");
	YAMLWorldSerializer s(world);
	auto str = s.serializeToString();
	std::ofstream ofs("world.yaml");
	ofs << str;
}

void Game::cursorManipFunc()
{
	Vec2 worldCoord = renderer.getCamera().windowToWorld(in.getMousePosition());
	Vec2 worldVel = (cursorData.oldPos - worldCoord) * getDeltaTimeSafe();
	Transform b = Transform(worldCoord, 0);
	Collider c = Collider({ 0.02,0.02 }, Form::Circle);
	//renderer.submit(
	//	Drawable(0, worldCoord, 2.0f, Vec2(0.02, 0.02) / renderer.getCamera().zoom, Vec4(1, 0, 0, 1), Form::Circle, RotaVec2(0), //RenderSpace::WorldSpace),
	//	LAYER_FIRST_UI
	//);
	if (!cursorData.locked && in.buttonPressed(Button::MB_LEFT)) {
		std::vector<CollisionInfo> collisions;
		collisionSystem.checkForCollisions(collisions, Collider::DYNAMIC | Collider::SENSOR | Collider::STATIC | Collider::PARTICLE, b, c);
		if (!collisions.empty()) {
			EntityHandleIndex topEntity = collisions.front().indexB;
			EntityHandle id = world.getHandle(topEntity);
			cursorData.relativePos = world.getComp<Transform>(topEntity).position - worldCoord;
			cursorData.lockedID = id;
			cursorData.locked = true;
		}
	}
	else if (in.buttonPressed(Button::MB_LEFT)) {
		if (world.isHandleValid(cursorData.lockedID)) {
			world.getComp<Transform>(cursorData.lockedID).position = cursorData.relativePos + worldCoord;
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


	//EntityHandleIndex cursor = world.getIndex(cursorID);
	//auto& baseCursor = world.getComp<Base>(cursor);
	//auto& colliderCursor = world.getComp<Collider>(cursor);
	//baseCursor.position = getPosWorldSpace(getCursorPos());
	//
	//baseCursor.rotation = renderer.getCamera().rotation;
	//colliderCursor.size = Vec2(1, 1) / renderer.getCamera().zoom / 100.0f;
	//
	//for (auto ent : world.entity_view<Player>()) {
	//	renderer.getCamera().position = world.getComp<Base>(ent).position;
	//}
	//
	////world.getComp<Draw>(cursorID).scale = vec2(1, 1) / renderer.getCamera().zoom / 100.0f;
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