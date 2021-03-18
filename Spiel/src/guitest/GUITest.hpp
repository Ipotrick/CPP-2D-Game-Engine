#pragma once

#include "../engine/EngineCore.hpp"
#include "../engine/gui/GUIManager.hpp"

#include "../engine/rendering/pipeline/RenderRessourceManager.hpp"
#include "../engine/rendering/DefaultRenderer.hpp"

#include "../engine/rendering/Font.hpp"

#include "../engine/EventSystem.hpp"

class GUIApp : public EngineCore {
public:

	GUIApp() : EngineCore{ "GUI Test", 1000, 1000 } { }

	gui::Manager::RootHandle makeUI()
	{
		using namespace gui;

		auto makeDropBox = [&](u32 child = gui::INVALID_ELEMENT_ID) {
			return ui.build(DropBox{
				.minsize = &gridSize,
				.child = child,
			});
		};

		auto makeDraggi = [&]() {
			return ui.build(DragDroppable{
				.child = ui.build(Box{
					.minsize = Vec2{ 25,25 },
					.color = DEFAULT_STYLE.accent0,
					.padding = Padding{},
				})
			});
		};
		
		return ui.build(Root{
		.sizing		= { 250, 550 },
		.placing	= Placing().absDistTop(100).absDistLeft(100),
		.child		= ui.build(Box{
			.bFillSpace = true,
			.bDragable = true,
			.color = DEFAULT_STYLE.fill1,
			.child		= ui.build(HeadTail{
				.children = {
					ui.build(ScrollBox{
						.colorView = DEFAULT_STYLE.fill0,
						.colorScrollBar = DEFAULT_STYLE.fill2,
						.padding = Padding{5,5,5,5},
						.child= ui.build(Column{
							.onUpdate = [this](Group& self, u32 id) { self.xalign = (this->bCheckbox ? XAlign::Left : XAlign::Center); },
							.children = {
								ui.build(Box{
									.bFillSpace = true,
									.bDragable = true,
									.color = DEFAULT_STYLE.fill1,
									.xalign = XAlign::Center,
									.yalign = YAlign::Center,
									.cornerRounding = 0.0f,
									.child = ui.build(StaticText{.value = "GRAB AND DRAG ME", .color = Vec4{0.9,0.9,0.9,1}}),
								}),
								ui.build(Row{
									.packing = Packing::Spread,
									.spacing = 5,
									.children = {
										ui.build(Row{
											.children = {
												ui.build(Radiobox{.value = &radio1, .index = 0, .size = Vec2{25,25}}),
												ui.build(StaticText{.value = "radio0", .color = {1,1,1,1}}),
											}
										}),
										ui.build(Row{
											.children = {
												ui.build(Radiobox{.value = &radio1, .index = 1, .size = Vec2{25,25}}),
												ui.build(StaticText{.value = "radio1", .color = {1,1,1,1}}),
											}
										}),
										ui.build(Row{
											.children = {
												ui.build(Radiobox{.value = &radio1, .index = 2, .size = Vec2{25,25}}),
												ui.build(StaticText{.value = "radio2", .color = {1,1,1,1}}),
											}
										}),
									}
								}),
								ui.build(Box{.minsize = Vec2{5,5}, .bFillSpace = true, .color = DEFAULT_STYLE.fill1, .padding = Padding{}, .cornerRounding = 0.0f}),
								ui.build(Row{
									.spacing = 5,
									.children = {
										ui.build(Checkbox{.value = &bCheckbox, .size = Vec2{25,25}}),
										ui.build(StaticText{.value = "Left Align ", .color = {1,1,1,1}}),
									}
								}),
								ui.build(Button{
									.size = Vec2{ 150, 25 },
									.onHold = [this](Button& self) { ui.globalScaling += getDeltaTime() * 0.5f; },
									.child = ui.build(StaticText{.value = "Grow UI"}),
								}),
								ui.build(Button{
									.size = Vec2{ 150, 25 },
									.onHold = [this](Button& self) { ui.globalScaling -= getDeltaTime() * 0.5f; },
									.child = ui.build(StaticText{.value = "Shrink UI"}),
								}),
								// text input demo:
								ui.build(TextInput{
									.onUpdate = [this](TextInput& self, u32 id) { self.xalign = (bCheckbox ? XAlign::Left : XAlign::Center); },
									.onStore = [this](std::string str) { text = str; },
									.size = {190, 50},
									.bClearOnEnter = true,
								}),
								ui.build(Text{.value = &text, .color = {0.9,0.9,0.9,1}}),
								ui.build(Row{
									.xalign = XAlign::Center,
									.packing = Packing::Tight,
									.children = {
										ui.build(TextInputF64{.value = &mainSliderMin, .size = Vec2{45,20}}),
										ui.build(SliderF64{
											.value = &sliderValue,
											.size = Vec2{80,20},
											.min = &mainSliderMin,
											.max = &mainSliderMax,
											.bThin = false,
											.child = ui.build(Text{.value = &sliderText, .color = Vec4{1,1,1,1}})
										}),
										ui.build(TextInputF64{.value = &mainSliderMax, .size = Vec2{45,20}}),
									}
								}),
								// three sliders:
								ui.build(Row{
									.xalign = XAlign::Center,
									.packing = Packing::Uniform,
									.children = {
										ui.build(Column{
											.xalign = XAlign::Center,
											.children = {
												ui.build(Text{.value = &threeSlidersMainMinText, .fontSize = 12, .color = Vec4{1,1,1,1}}),
												ui.build(SliderF64{.value = &threeSlidersMainMin, .size = Vec2{20,100}, .min = 5.0, .max = 50.0, .bVertical = true}),
											}
										}),
										ui.build(Column{
											.xalign = XAlign::Center,
											.children = {
												ui.build(Text{.value = &threeSlidersMainMaxText, .fontSize = 12, .color = Vec4{1,1,1,1}}),
												ui.build(SliderF64{.value = &threeSlidersMainMax, .size = Vec2{20,100}, .min = 5.0, .max = 50.0, .bVertical = true}),
											}
										}),
										ui.build(Column{
											.xalign = XAlign::Center,
											.children = {
												ui.build(Text{.value = &threeSlidersMainValueText, .fontSize = 12, .color = Vec4{1,1,1,1}}),
												ui.build(SliderF64{
													.value = &threeSlidersMainValue,
													.size = Vec2{20,100},
													.min = &threeSlidersMainMin,
													.max = &threeSlidersMainMax,
													.bVertical = true,
													.colorSlider = Vec4{0.5,0.5,1,1},
												}),
											}
										}),
									}
								}),
								ui.build(Row{
									.xalign = XAlign::Center,
									.yalign = YAlign::Center,
									.packing = Packing::Uniform,
									.children = {
										makeDropBox(),
										makeDropBox(),
										makeDropBox(),
										makeDropBox(),
									}
								}),
								ui.build(Row{
									.xalign = XAlign::Center,
									.yalign = YAlign::Center,
									.packing = Packing::Uniform,
									.children = {
										makeDropBox(),
										makeDropBox(),
										makeDropBox(),
										makeDropBox(),
									}
								}),
								ui.build(Row{
									.xalign = XAlign::Center,
									.yalign = YAlign::Center,
									.packing = Packing::Uniform,
									.children = {
										makeDropBox(),
										makeDropBox(),
										makeDropBox(makeDraggi()),
										makeDropBox(makeDraggi())
									}
								}),
							}
						}),
					}),
					
					// Footer:
					ui.build(Box{ 
						.bFillSpace = true, 
						.color = DEFAULT_STYLE.fill1,
						.xalign = XAlign::Right, 
						.yalign = YAlign::Center, 
						.child = ui.build(Box{.minsize = Vec2{20,20},.bDragable = true,  .color = DEFAULT_STYLE.accent0, .onDrag = OnDrag::Resize })
					}),
					}
				}),
			}),
		});
	}

	std::string genDeltaTimeStr()
	{
		return std::string("deltatime: ") + std::to_string(this->getDeltaTime());
	}


	virtual void create() override final
	{
		renderer.init(&mainWindow);
		renderer.supersamplingFactor = 2;
		handle = makeUI();

		using namespace gui;

		ui.build(Root{
			.sizing = Sizing{}.absX(100).absY(100),
			.placing = Placing{}.absDistLeft(100).absDistTop(100),
			.child = ui.build(Box{
				.bFillSpace = true,
				.bDragable = true,
				.xalign = XAlign::Center,
				.yalign = YAlign::Center,
				.child = ui.build(StaticText{.value="Test Text"}),
			})
		});
	}

	virtual void update(float deltaTime) override final
	{
		for (f32 angle = 0.0f; angle < 360.0f; angle += 1.0f) {
			renderer.drawSprite(Sprite{ .position = rotate(Vec2{0.5f,0.0f},angle), .scale = Vec2{0.005f,0.005f} });
		}
		threeSlidersMainMinText = std::string("min:") + std::to_string(threeSlidersMainMin);
		threeSlidersMainMaxText = std::string("max:") + std::to_string(threeSlidersMainMax);
		threeSlidersMainValueText = std::string("size:") + std::to_string(threeSlidersMainValue);
		sliderText = std::string("FPS: ") + std::to_string(sliderValue);
		gridSize.x = threeSlidersMainValue;
		gridSize.y= threeSlidersMainValue;
		setMaxFPS(std::max(5.0, sliderValue));
		ui.draw(renderer.getCoordSys(), &renderer.tex, &renderer.fonts, mainWindow, deltaTime);
		renderer.drawUISprites(ui.getSprites());
		if (mainWindow.keyJustPressed(Key::G)) {
			TextureLoadInfo desc{ "ressources/ConsolasSoft.png" };
			renderer.tex.clear();
		}
		if (mainWindow.keyJustPressed(Key::A)) {
			if (ui.isHandleValid(handle)) {
				ui.destroy(handle);
			}
		}
		if (mainWindow.keyJustPressed(Key::S)) {
			if (!ui.isHandleValid(handle)) {
				handle = makeUI();
			}
		}
		if (mainWindow.keyPressed(Key::NP_ADD)) { ui.globalScaling *= 1.0f + deltaTime * 0.1f; }
		if (mainWindow.keyPressed(Key::NP_SUBTRACT)) { ui.globalScaling /= 1.0f + deltaTime * 0.1f; }
		renderer.start();

	}

	virtual void destroy() override final
	{
	}

private:
	DefaultRenderer renderer;

	gui::Manager ui;
	gui::Manager::RootHandle handle;
	bool bCheckbox{ false };
	std::string text;
	Vec2 gridSize{ 25,25 };
	u32 radio1{ 0 };

	std::string sliderText;
	f64 sliderValue{ 9999.0 };
	f64 mainSliderMin{ 5.0 };
	f64 mainSliderMax{ 9999.0 };

	// threeSliders:
	f64 threeSlidersMainMax{ 0.0 };
	std::string threeSlidersMainMaxText;
	f64 threeSlidersMainMin{ 0.0 };
	std::string threeSlidersMainMinText;
	f64 threeSlidersMainValue{ 0.0 };
	std::string threeSlidersMainValueText;
};
