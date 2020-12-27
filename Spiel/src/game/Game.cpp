#include "Game.hpp"

#include <iomanip>

#include "../engine/util/Log.hpp"
#include "GameComponents.hpp"
#include "serialization/YAMLSerializer.hpp"

#include "transformScript.hpp"
#include "movementScript.hpp"
#include "drawScript.hpp"
#include "LayerConstants.hpp"
#include "BasicScripts.hpp"
#include "ParticleScript.hpp"
#include "SuckerScript.hpp"
#include "TesterScript.hpp"
#include "../engine/ui/UICreate.hpp"

#include "BloomRScript.hpp"

#include "LoadBallTestMap.hpp"
#include "LoadRenderTestMap.hpp"

using namespace util;

Game::Game()
{
	initialize("Balls2", 1600, 900);

	renderer.setLayerCount(LAYER_MAX);

	renderer.getLayer(LAYER_WORLD_BACKGROUND).bClearEveryFrame = true;

	renderer.getLayer(LAYER_WORLD_MIDGROUND).renderMode = RenderSpace::WorldSpace; 
	renderer.getLayer(LAYER_WORLD_MIDGROUND).depthTest = DepthTest::Less;

	renderer.getLayer(LAYER_WORLD_PARTICLE).renderMode = RenderSpace::WorldSpace;

	renderer.getLayer(LAYER_WORLD_FOREGROUND).renderMode = RenderSpace::WorldSpace;

	renderer.getLayer(LAYER_WORLD_POSTPROCESS).renderMode = RenderSpace::WorldSpace;
	renderer.getLayer(LAYER_WORLD_POSTPROCESS).attachRenderScript(std::make_unique<BloomRScript>());

	renderer.getLayer(LAYER_DEBUG_UI).renderMode = RenderSpace::WorldSpace;

	renderer.getLayer(LAYER_FIRST_UI).renderMode = RenderSpace::PixelSpace;

	renderer.getLayer(LAYER_SECOND_UI).renderMode = RenderSpace::PixelSpace;

	collisionSystem.disableColliderDetection(Collider::PARTICLE);
}

#define UI_CREATE(parent, code, Type, name) \
{ \
	Type name; \
	code \
	parent.addChild(EngineCore::ui.createAndGetPtr(name)); \
}

#define UI_TEXT(parent, code)		UI_CREATE(parent, code, UIText, me)

#define UI_SEPERATOR(parent, code)	UI_CREATE(parent, code, UISeperator, me)

#define UI_TEXT_UPDATE(code) \
me.setUpdateFn([&](UIElement* e){ UIText& me = *((UIText*)e); code });

void Game::create() {


	world.setOnRemCallback<Health>(onHealthRemCallback);

	auto size = getWindowSize();
	renderer.getCamera().frustumBend = (Vec2(1 / getWindowAspectRatio(), 1.0f));
	renderer.getCamera().zoom = 1 / 3.5f;

	const float firstRowWidth = 90.0f;
	const Vec2 textFieldSize{ firstRowWidth , 17.0f };
	const auto font = renderer.makeSmallTexRef(TextureInfo("ConsolasAtlas2.png"));

	//loadBallTestMap();

	std::ifstream ifstream("world.yaml");
	if (ifstream.good()) {
		YAMLWorldSerializer s(world);
		std::string str;
		std::getline(ifstream, str, '\0');
		s.deserializeString(str);
	}

	auto makeRenderStatsUI = [&](auto& parent) {
		UICollapsable c("Rendering Statics:", font);
		c.setHeadLength(20);
		{
			UIList8 l;
			l.setSizeModeY(SizeMode::FillMin);
			l.setSizeModeX(SizeMode::FillUp);
			l.setPadding({ 5.0f, 0.0f });
			{
				UIPair uicountPair;
				uicountPair.setHorizontal();
				{
					UIText uiCountText("ui count:", font);
					uiCountText.setSize(textFieldSize);
					uicountPair.setFirst(ui.createAndGetPtr(uiCountText));
				}
				{
					UIText uiCountText2("", font, [&](UIElement* e) {
						((UIText*)e)->text = std::to_string(ui.activeElementCount());
						}
					);
					uiCountText2.setSize(textFieldSize);
					uicountPair.setSecond(ui.createAndGetPtr(uiCountText2));
				}
				l.addChild(ui.createAndGetPtr(uicountPair));
			}
			{
				UIPair uidcountPair;
				uidcountPair.setHorizontal();
				{
					UIText uidCountText("ui draw:", font);
					uidCountText.setSize(textFieldSize);
					uidcountPair.setFirst(ui.createAndGetPtr(uidCountText));
				}
				{
					UIText uidCountText2("", font, [&](UIElement* e) { ((UIText*)e)->text = std::to_string(ui.drawCount()); });
					uidCountText2.setSize(textFieldSize);
					uidcountPair.setSecond(ui.createAndGetPtr(uidCountText2));
				}
				l.addChild(ui.createAndGetPtr(uidcountPair));
			}
			{
				UIPair drawCallPair;
				drawCallPair.setHorizontal();
				{
					UIText drawcallText("Drawcalls:", font);
					drawcallText.setSize(textFieldSize);
					drawCallPair.setFirst(ui.createAndGetPtr(drawcallText));
				}
				{
					UIText drawcallText2("", font, [&](UIElement* e) { ((UIText*)e)->text = std::to_string(renderer.getDrawCalls()); });
					drawcallText2.setSize(textFieldSize);
					drawCallPair.setSecond(ui.createAndGetPtr(drawcallText2));
				}
				l.addChild(ui.createAndGetPtr(drawCallPair));
			}

			c.addChild(ui.createAndGetPtr(l));
		}
		parent.addChild(ui.createAndGetPtr(c));
	};

	// ui render stats ui:
	{
		UIFrame frame;
		frame.layer = LAYER_SECOND_UI;
		frame.setWidth(200);
		frame.setPadding({ 5.0f, 5.0f });
		frame.anchor.setLeftAbsolute(10);
		frame.anchor.setTopAbsolute(10);
		{
			UIList16 list;
			list.setSizeModeX(SizeMode::FillUp);
			list.setSizeModeY(SizeMode::FillMin);
			list.setSpacing(5.0f);
			list.addChild(
				ui::text({
				.anchor = UIAnchor({
						.xmode = UIAnchor::X::LeftRelativeDist,
						.x = 0.5
					}),
				.size = textFieldSize,
				.textAnchor = UIAnchor({
						.xmode = UIAnchor::X::LeftRelativeDist,
						.x = 0.5
					}),
				.text = "Statistics:",
				.fontTexture = font
				}
			));
			UI_SEPERATOR(list, me.setHorizontal();)
			{
				UIPair entCountPair;
				entCountPair.setHorizontal();
				{
					UIText entitiesText("Entities:", font);
					entitiesText.setSize(textFieldSize);
					entCountPair.setFirst(ui.createAndGetPtr(entitiesText));
				}
				{
					UIText entitiesText2("", font, [&](UIElement* e) { ((UIText*)e)->text = std::to_string(world.size()); });
					entitiesText2.setSize(textFieldSize);
					entCountPair.setSecond(ui.createAndGetPtr(entitiesText2));
				}
				list.addChild(ui.createAndGetPtr(entCountPair));
			}
			makeRenderStatsUI(list);
			{
				UIPair ticksPair;
				ticksPair.setHorizontal();
				{
					UIText ticksText("Ticks/s:", font);
					ticksText.setSize(textFieldSize);
					ticksPair.setFirst(ui.createAndGetPtr(ticksText));
				}
				{
					UIText ticksText2("", font, [&](UIElement* e) { ((UIText*)e)->text = std::to_string(1.0f / getDeltaTime(100)); });
					ticksText2.setSize(textFieldSize);
					ticksPair.setSecond(ui.createAndGetPtr(ticksText2));
				}
				list.addChild(ui.createAndGetPtr(ticksPair));
			}
			frame.addChild(ui.createAndGetPtr(list));
		}
		ui.createFrame(frame, "Statiscics");
	}
}

void Game::update(float deltaTime) {
	if (bLoading) {
		if (JobSystem::finished(loadingWorkerTag)) {
			Monke::log("job with tag {0} finished clientside", (uint32_t)loadingWorkerTag);
			bLoading = false;
			in.returnFocus();
			in.returnMouseFocus();
			world = loadedWorld;
			if (ui.doesFrameExist("loadingtext")) {
				ui.destroyFrame("loadingtext");
			}
			ui.update();
		}
	}
	else {
		renderer.submit(makeSprite(0, { 0,0 }, -1.0f, { 2, 2 }, { 0.1, 0.1, 0.1f, 0.1f }, Form::Rectangle, RotaVec2{ 0 }, RenderSpace::WindowSpace), LAYER_WORLD_BACKGROUND);
		{
			collisionSystem.execute(world.submodule<COLLISION_SECM_COMPONENTS>(), deltaTime);
			for (auto& d : collisionSystem.getDebugSprites()) renderer.submit(d, LAYER_WORLD_FOREGROUND);
			physicsSystem2.execute(world.submodule<COLLISION_SECM_COMPONENTS>(), world.physics, deltaTime, collisionSystem);
			for (auto& d : physicsSystem2.getDebugSprites()) renderer.submit(d,LAYER_WORLD_FOREGROUND);

			struct LJob : public IJob {
				std::function<void(uint32_t)> function;
				LJob(std::function<void(uint32_t)> f):
					function{ f }
				{ }

				void execute(uint32_t thread) override { function(thread); }
			};

			/* UNSAFE BEGIN */

			std::vector<LJob> jobList;
			jobList.push_back(LJob{
				[&](uint32_t thread) {
					for(auto [ent, t] : world.entityComponentView<Transform>())
					{
						transformScript(ent, t);
					}
				} 
			});
			jobList.push_back(LJob{
				[&, deltaTime](uint32_t thread) {
					for (auto [ent, m, t] : world.entityComponentView<Movement, Transform>()) {
						movementScript(ent, t, m, deltaTime);
					}
				}
			});
			JobSystem::wait(JobSystem::submitVec(std::move(jobList)));

			/* UNSAFE END */

			//for (auto [ent, t] : world.entityComponentView<Transform>()) {
			//	transformScript(ent, t);
			//	if (world.hasComp<Movement>(ent)) movementScript(ent, t, world.getComp<Movement>(ent), deltaTime);
			//}
		}

		in.manualUpdate();

		gameplayUpdate(deltaTime);

		for (auto [ent, t, d] : world.entityComponentView<Transform, Draw>()) drawScript(ent, t, d);

		world.update();
	}
}

void Game::gameplayUpdate(float deltaTime)
{
	if (in.keyPressed(Key::NP_6, Focus::Global)) {
		world.physics.linearEffectDir = { 1, 0 };
		renderer.getCamera().rotation = 90;
	}
	if (in.keyPressed(Key::NP_8, Focus::Global)) {
		world.physics.linearEffectDir = { 0, 1 };
		renderer.getCamera().rotation = 180;
	}
	if (in.keyPressed(Key::LEFT_ALT, Focus::Global) && in.keyPressed(Key::F4, Focus::Global)) {
		EngineCore::quit();
	}
	if (in.keyPressed(Key::G)) {
		world.physics.linearEffectAccel += 8 * deltaTime;
	}
	if (in.keyJustPressed(Key::P) && !in.keyPressed(Key::LEFT_SHIFT)) {
		renderer.getCamera().position = { 0,0 };
		world = World();
		loadRenderTestMap(0.5f);
	}
	if (in.keyJustPressed(Key::P) && in.keyPressed(Key::LEFT_SHIFT)) {
		renderer.getCamera().position = { 0,0 };
		world = World();
		loadRenderTestMap(0.3f);
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
		if (ui.doesFrameExist("Statiscics")) {
			ui.getFrame("Statiscics").disable();
		}
	}
	if (in.keyJustPressed(Key::B) && in.keyPressed(Key::LEFT_SHIFT)) {
		if (ui.doesFrameExist("Statiscics")) {
			ui.getFrame("Statiscics").enable();
		}
	}
	if (in.keyPressed(Key::PERIOD, Focus::Global)) {
		renderer.getLayer(LAYER_WORLD_BACKGROUND).detachRenderScript();
	}
	if (in.keyPressed(Key::I)) {
		uiContext.scale = clamp(uiContext.scale - deltaTime, 0.1f, 10.0f);
	}
	if (in.keyPressed(Key::O)) {
		uiContext.scale = clamp(uiContext.scale + deltaTime, 0.1f, 10.0f);
	}
	if (in.keyPressed(Key::J)) {
		world = World();
		loadBallTestMap();
		ui.update();
	}
	if (in.keyJustPressed(Key::K)) {

		class SaveJob : public IJob {
		public:
			SaveJob(World w):
				w{ std::move(w) }
			{ }
			virtual void execute(const uint32_t thread) override
			{
				Monke::log("Start saving...");

				std::ofstream of("world.yaml");
				if (of.good()) {
					YAMLWorldSerializer s(w);
					of << s.serializeToString();
					of.close();
				}
				Monke::log("Finished saving!");
			}
		private:
			World w;
		};

		auto tag = JobSystem::submit(SaveJob(world));
		JobSystem::orphan(tag);
	}
	if (in.keyJustPressed(Key::L)) {
		bLoading = true;
		in.takeFocus(Focus::UI);
		in.takeMouseFocus(Focus::UI);

		class LoadJob : public IJob {
		public:
			LoadJob(World& loadedWorld): loadedWorld{ loadedWorld } {}

			virtual void execute(uint32_t const thread) override
			{
				Monke::log("Start loading...");
				loadedWorld = World();
				std::ifstream ifstream("world.yaml");
				if (ifstream.good()) {
					YAMLWorldSerializer s(loadedWorld);
					std::string str;
					std::getline(ifstream, str, '\0');
					s.deserializeString(str);
				}
				Monke::log("Finished loading!");
			}
		private:
			World& loadedWorld;
		};

		loadingWorkerTag = JobSystem::submit(LoadJob(loadedWorld));

		ui::frame("loadingtext",
			UIFrame::Parameters{
			.anchor = UIAnchor(UIAnchor::Parameters{
				.xmode = UIAnchor::X::LeftRelativeDist,
				.x = 0.5,
				.ymode = UIAnchor::Y::TopRelativeDist,
				.y = 0.5,}),
			.size = {1000, 300},
			.layer = LAYER_FIRST_UI,
			.borders = {10,10}},
		{
			ui::text({
				.size = {1000, 300},
				.textAnchor = UIAnchor(UIAnchor::Parameters{
					.xmode = UIAnchor::X::LeftRelativeDist,
					.x = 0.5,
					.ymode = UIAnchor::Y::TopRelativeDist,
					.y = 0.5,}),
				.text = "Loading ...",
				.fontTexture = renderer.makeSmallTexRef(TextureInfo("ConsolasAtlas.png")),
				.fontSize = {30,100}
			})
		}
		);
	}

	//execute scripts
	for (auto [ent, comp] : world.entityComponentView<Health>()) healthScript(ent, comp, deltaTime);
	for (auto [ent, comp] : world.entityComponentView<Player>()) playerScript(ent, comp, deltaTime);
	for (auto [ent, comp] : world.entityComponentView<Age>()) ageScript(ent, comp, deltaTime);
	for (auto [ent, comp] : world.entityComponentView<Bullet>()) bulletScript(ent, comp, deltaTime);
	for (auto [ent, comp] : world.entityComponentView<ParticleScriptComp>()) particleScript(ent, comp, deltaTime);
	for (auto [ent, comp] : world.entityComponentView<SuckerComp>()) suckerScript(ent, comp, deltaTime);
	for (auto [ent, comp] : world.entityComponentView<Tester>()) testerScript(ent, comp, deltaTime);

	cursorManipFunc();

	//for (auto ent : world.entityView<SpawnerComp>()) {
	//	const Transform base = world.getComp<Transform>(ent);
	//	int laps = spawnerLapTimer.getLaps(deltaTime);
	//	for (int i = 1; i < laps; i++) {
	//		float rotation = (float)(rand() % 360);
	//		auto particle = world.create();
	//		Vec2 movement = rotate(Vec2(5,0), rotation);
	//		world.addComp<Transform>(particle, Transform(base.position));
	//		auto size = Vec2(0.36, 0.36) * ((rand() % 1000) / 1000.0f);
	//		float gray = (rand() % 1000 / 1000.0f);
	//		world.addComp<Draw>(particle, Draw(Vec4(gray, gray, gray, 0.3), size, rand() % 1000 / 1000.0f, Form::Circle));
	//		world.addComp<Movement>(particle, Movement(movement, rand()%10000/100.0f -50.0f));
	//		//world.addComp<Collider>(particle, Collider(size, Form::Circle, true));
	//		//world.addComp<PhysicsBody>(particle, PhysicsBody(1, 0.01, 10, 0));
	//		world.addComp<Age>(particle, Age(rand()%1000/2000.0f*3));
	//		world.spawn(particle);
	//	}
	//}

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
	//	Sprite(0, worldCoord, 2.0f, Vec2(0.02, 0.02) / renderer.getCamera().zoom, Vec4(1, 0, 0, 1), Form::Circle, RotaVec2(0), //RenderSpace::WorldSpace),
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