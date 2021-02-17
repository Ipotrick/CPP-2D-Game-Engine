#pragma once

#include "../engine/EngineCore.hpp"
#include "../engine/rendering/DefaultRenderer.hpp"
#include "../engine/entity/EntityComponentManager.hpp"
#include "../engine/gui/GUIManager.hpp"

#include "AntsWorld.hpp"

/// <summary>
/// TODO BETTER UI:
/// </summary>

class AntsApp : public EngineCore {
public:

	AntsApp() : EngineCore{ "Ants", 1000, 1000 }
	{
		world.renderer.init(&mainWindow);
		world.renderer.camera.zoom /= 600;
		world.renderer.supersamplingFactor = 1;
		world.collsys.disableColliderDetection(Collider::PARTICLE);
	}

	gui::Manager::RootHandle makeUI()
	{
		using namespace gui;
		auto generateNestColumn = [&](Group& self, u32 selfid) {
			for (u32 i = self.children.size() - 1; i >= 6; i--) {
				ui.destroy(self.children.at(i));
				self.children.pop_back();
			}
			for (auto [ent, nest] : world.ecm.entityComponentView<Nest>()) {
				std::string text = std::string("Foodcount: ") + std::to_string(nest.amount);
				self.children.push_back(
					ui.build(Text{.value = text , .color = Vec4{0,0,0,1} })
				);
			}
			ui.updateChildHierarchy(selfid);
		};


		return ui.build(Root{
			.sizing = Sizing{}.absX(180).absY(100),
			.placing = Placing{}.absDistLeft(20).absDistTop(20),
			.child = ui.build(Box{
				.color = Vec4{0.8,0.8,0.8,1},
				.bDragable = true,
				.child = ui.build(Column{
					.onUpdate = generateNestColumn,
					.children = {
						ui.build(SliderF64{
							.value=&timewarp, 
							.min=0.1, 
							.max=30.0, 
							.bThin=false, 
							.child = ui.build(Text{.onUpdate = [&](Text& self, u32 id) {self.value = std::string("timewarp: ") + std::to_string(timewarp); }, .value = std::string("")})
						}),
						ui.build(SliderF64{
							.value = &antcount,
							.min = 0.0,
							.max = 10000.0,
							.bThin = false,
							.child = ui.build(Text{
								.onUpdate = [&](Text& self, u32 id) {self.value = std::string("antcount: ") + std::to_string(antcount); }, 
								.value = std::string(""),
							})
						}),
						ui.build(Row{
							.children = {
								ui.build(Checkbox{.value = &bPheroVisible}),
								ui.build(StaticText{.value = "enable phero view:", .color = Vec4{0,0,0,1}}),
							}
						}),
						ui.build(Row{
							.children = {
								ui.build(Checkbox{.value = &DrawFoodOrNestPhero}),
								ui.build(StaticText{.value = "food or nest phero:", .color = Vec4{0,0,0,1}}),
							}
						}),
						ui.build(Row{
							.children = {
								ui.build(Checkbox{.value = &bPheroTimeOrIntensity}),
								ui.build(StaticText{.value = "phero time or intensity:", .color = Vec4{0,0,0,1}}),
							}
						}),
						ui.build(StaticText{.value = "Food in Nests:", .color = Vec4{0,0,0,1}}),
					}
				})
			})
		});
	}

	void create() override { 
		for (s32 i = 0; i < 2000; i++) {
			world.spawnAnt(Transform{ Vec2{0,0}, RotaVec2{ f32(rand() % 360) } }, 20);
		}

		nestui = makeUI();

		const auto nest = world.ecm.create();
		world.ecm.addComp(nest, Transform{ Vec2{0,0} });
		world.ecm.addComp(nest, Collider{ Vec2{30,30} });
		world.ecm.addComp(nest, PhysicsBody{});
		world.ecm.addComp(nest, Nest{});
		world.ecm.spawn(nest);
	}

	void destroy() override { }

	void update(f32 deltaTime) override {
		ui.globalScaling = std::floorf(uiscale*12.0f)/12.0f;
		ui.draw(world.renderer.getCoordSys(), &world.renderer.tex, &world.renderer.fonts, mainWindow, deltaTime);
		world.renderer.drawUISprites(ui.getSprites());

		antcount = std::floorf(antcount);
		if (antcount > oldAntCount) {
			f64 diff = antcount - oldAntCount;
			for (u32 i = 0; i < diff; i++) {
				world.spawnAnt(Transform{ Vec2{0,0}, RotaVec2{ f32(rand() % 360) } }, 20);
			}
		}
		else if (antcount < oldAntCount) {
			f64 diff = oldAntCount - antcount;
			for (auto ent : world.ecm.entityView<Ant>()) {
				if (diff < 0.0f) break;
				world.ecm.destroy(ent);
				diff--;
			}
		}
		oldAntCount = antcount;

		if (mainWindow.keyPressed(Key::NP_8)) {
			world.renderer.camera.position -= rotate(Vec2(0.0f, -1.0f) * 1.0f / world.renderer.camera.zoom, world.renderer.camera.rotation) * deltaTime;
		}
		if (mainWindow.keyPressed(Key::NP_4)) {
			world.renderer.camera.position -= rotate(Vec2(1.0f, 0.0f) * 1.0f / world.renderer.camera.zoom, world.renderer.camera.rotation) * deltaTime;
		}
		if (mainWindow.keyPressed(Key::NP_2)) {
			world.renderer.camera.position -= rotate(Vec2(0.0f, 1.0f) * 1.0f / world.renderer.camera.zoom, world.renderer.camera.rotation) * deltaTime;
		}
		if (mainWindow.keyPressed(Key::NP_6)) {
			world.renderer.camera.position -= rotate(Vec2(-1.0f, 0.0f) * 1.0f/world.renderer.camera.zoom, world.renderer.camera.rotation) * deltaTime;
		}
		if (mainWindow.keyPressed(Key::NP_7)) {
			world.renderer.camera.rotation += 100.0f * deltaTime;
		}
		if (mainWindow.keyPressed(Key::NP_9)) {
			world.renderer.camera.rotation -= 100.0f * deltaTime;
		}
		if (mainWindow.keyPressed(Key::NP_ADD)) {
			world.renderer.camera.zoom *= 1.0f + (1.0f * deltaTime);
		}
		if (mainWindow.keyPressed(Key::NP_SUBTRACT)) {
			world.renderer.camera.zoom *= 1.0f - (1.0f * deltaTime);
		}
		if (mainWindow.keyPressed(Key::O)) {
			uiscale *= 1.0f + deltaTime;
			uiscale = std::min(uiscale, 10.0f);
		}
		if (mainWindow.keyPressed(Key::P)) {
			uiscale /= 1.0f + deltaTime;
			uiscale = std::max(uiscale, 1.0f);
		}
		if (mainWindow.buttonPressed(MouseButton::MB_LEFT)) {
			const Vec2 cursorWorldPos = world.renderer.getCoordSys().convertCoordSys<RenderSpace::Window, RenderSpace::Camera>(mainWindow.getCursorPos());

			u32 laps = footSpawnLapTimer.getLaps(deltaTime);

			for (u32 i = 0; i < laps; i++) {
				world.spawnFood(cursorWorldPos,1000);
			}
		}
		if (mainWindow.buttonPressed(MouseButton::MB_RIGHT)) {
			const Vec2 cursorWorldPos = world.renderer.getCoordSys().convertCoordSys<RenderSpace::Window, RenderSpace::Camera>(mainWindow.getCursorPos());

			if (barrierSpawnLapTimer.getLaps(deltaTime) > 0) {
				world.spawnBarrier(cursorWorldPos, Vec2{ 30,30 });
			}
		}

		deltaTime *= timewarp;

		LogTimer t1(std::cout, "collision update:");
		world.collsys.execute(world.ecm.submodule<COLLISION_SECM_COMPONENTS>(), deltaTime);
		t1.stop();
		world.updateAnts(deltaTime);
		world.updateFood(deltaTime);
		world.updateNests(deltaTime);
		LogTimer t(std::cout, "phero grid update:");
		world.nestPheromone.update(deltaTime);
		world.foodTransportPherogrid.update(deltaTime);
		t.stop();
		if (bPheroVisible) {
			std::vector<Sprite> gridsprites;
			if (DrawFoodOrNestPhero) {
				gridsprites = world.nestPheromone.draw({ 1,0,0,1 }, bPheroTimeOrIntensity);
			}
			else {
				gridsprites = world.foodTransportPherogrid.draw({ 0,1,0,1 }, bPheroTimeOrIntensity);
			}
			world.renderer.drawSprites(gridsprites);
		}
		world.ecm.update();

		for (auto ant : world.ecm.entityView<Ant, Transform>()) {
			drawAnt(world.ecm.getComp<Transform>(ant), world.ecm.getComp<Ant>(ant));
		}
		//for (auto p : world.ecm.entityView<Pheromone, Transform>()) {
		//	drawPheromone(world.ecm.getComp<Transform>(p), world.ecm.getComp<Pheromone>(p));
		//}
		for (auto d : world.ecm.entityView<Food, Transform>()) {
			drawFood(world.ecm.getComp<Transform>(d), world.ecm.getComp<Food>(d));
		}
		for (auto n : world.ecm.entityView<Nest, Transform>()) {
			drawNest(world.ecm.getComp<Transform>(n), world.ecm.getComp<Nest>(n));
		}
		for (auto n : world.ecm.entityView<Barrier, Transform>()) {
			drawBarrier(world.ecm.getComp<Transform>(n), world.ecm.getComp<Collider>(n));
		}
		world.renderer.start();
	}

	void drawBarrier(Transform const& t, Collider const& coll)
	{
		world.renderer.drawSprite(
			Sprite{
				.color = Vec4{0.2,0.2,0.2,1},
				.position = Vec3{t.position, -0.1f},
				.rotationVec = t.rotaVec,
				.scale = coll.size,
				.cornerRounding = coll.size.x * 0.5f,
				.drawMode = RenderSpace::Camera
			}
		);
	}

	void drawAnt(Transform const& t, Ant const& a)
	{
		//world.renderer.drawSprite(
		//	Sprite{
		//		.color = Vec4{0.3,0.3,0.3,1},
		//		.position = Vec3{t.position, -0.3f},
		//		.scale = Vec2{a.viewRange,a.viewRange},
		//		.cornerRounding = a.viewRange * 0.5f,
		//		.drawMode = RenderSpace::Camera
		//	}
		//);
		world.renderer.drawSprite(
			Sprite{
				.color = Vec4{0.8,0.2,0.2,1},
				.position = Vec3{t.position, -0.5f},
				.rotationVec = t.rotaVec,
				.scale = a.size,
				.drawMode = RenderSpace::Camera
			}
		);
		if (a.footAmountTransporing > 0.0f) {
			world.renderer.drawSprite(
				Sprite{
					.color = Vec4{0.3,0.7,0.2,1},
					.position = Vec3{t.position + t.rotaVec.toUnitX0()*0.5f, -0.5f},
					.rotationVec = t.rotaVec,
					.scale = Vec2{a.footAmountTransporing,a.footAmountTransporing} * 2.0f,
					.cornerRounding = a.footAmountTransporing * 0.5f * 2.0f,
					.drawMode = RenderSpace::Camera
				}
			);
		}
	}

	void drawFood(Transform const& t, Food const& f)
	{
		const f32 size = sqrt(f.amount);
		world.renderer.drawSprite(
			Sprite{
				.color = Vec4{0.3,0.7,0.2,1},
				.position = Vec3{t.position, -0.4},
				.rotationVec = t.rotaVec,
				.scale = Vec2{size,size},
				.cornerRounding = size * 0.5f,
				.drawMode = RenderSpace::Camera,
			}
		);
	}

	void drawNest(Transform const& t, Nest const& nest)
	{
		world.renderer.drawSprite(
			Sprite{
				.color = Vec4{0.2,0.3,0.5,1},
				.position = Vec3{t.position, -0.4},
				.rotationVec = t.rotaVec,
				.scale = Vec2{30,30},
				.cornerRounding = 15.0f,
				.drawMode = RenderSpace::Camera,
			}
		);
	}

private:
	f32 uiscale{ 1.0f };
	f64 timewarp{ 1.0 };
	f64 antcount{ 2000.0 };
	f64 oldAntCount{ 2000.0 };
	bool DrawFoodOrNestPhero{ true };
	bool bPheroTimeOrIntensity{ true };
	bool bPheroVisible{ false };


	std::string nestText;
	AntsWorld world;

	std::vector<f32> grid;

	LapTimer footSpawnLapTimer{ 0.1 };
	LapTimer barrierSpawnLapTimer{ 0.05 };
	gui::Manager ui;
	gui::Manager::RootHandle nestui;
};
