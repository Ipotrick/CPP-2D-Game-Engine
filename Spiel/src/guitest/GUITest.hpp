﻿#pragma once

#include "../engine/EngineCore.hpp"
#include "../engine/gui/GUIManager.hpp"

#include "../engine/rendering/pipeline/RenderRessourceManager.hpp"
#include "../engine/rendering/pipeline/OpenGLTexture2.hpp"

using TextureManager = RenderRessourceManager<TextureHandle, TextureDescriptor2, OpenGLTexture2>;

class GUIApp : public EngineCore {
public:
#define GET_MEMBER(x) [this]() { return this->x; }

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
		.placing	= Placing().relDistTop(0.5).absDistLeft(50),
		.child		= ui.build(Box{
			.bFillSpace = true,
			.bDragable	= true,
			.child = ui.build(Footer{
				.children = {
					ui.build(Column{
					.onUpdate = [this](Group& self) { self.xalign = (this->bCheckbox ? XAlign::Left : XAlign::Center); },
					.children = {
						ui.build(Box{
							.bFillSpace = true,
							.color = DEFAULT_STYLE.fill1,
							.xalign = XAlign::Center,
							.yalign = YAlign::Center,
							.bDragable = true,
							.child = ui.build(StaticText{.value = "GRAB AND DRAG ME", .color = Vec4{0.9,0.9,0.9,1}}),
						}),
						ui.build(Box{.minsize = Vec2{5,5}, .bFillSpace = true, .color = DEFAULT_STYLE.fill1, .padding = Padding{}}),
						ui.build(Row{
							.spacing = 5,
							.children = {
								ui.build(Checkbox{.value = &bCheckbox, .size = Vec2{25,25}}),
								ui.build(StaticText{.value = "Left Align ", .color = {1,1,1,1}}),
							}
						}),
						ui.build(Button{
							.size = Vec2{ 150, 25 },
							.text = StaticText{.value = "Grow UI"},
							.onHold = [this](Button& self) { ui.globalScaling += getDeltaTime() * 0.5f; },
						}),
						ui.build(Button{
							.size = Vec2{ 150, 25 },
							.text = StaticText{.value = "Shrink UI"},
							.onHold = [this](Button& self) { ui.globalScaling -= getDeltaTime() * 0.5f; },
						}),
						// text input demo:
						ui.build(TextInput{
							.onUpdate = [this](TextInput& self) { self.xalign = (bCheckbox ? XAlign::Left : XAlign::Center); },
							.size = {190, 50},
							.onStore = [this](std::string str) { text = str; },
							.bClearOnEnter = true,
						}),
						ui.build(Text{.value = &text, .color = {0.9,0.9,0.9,1}}),
						// fps slider:
						ui.build(Text{.value = &deltaTimeText, .color = {0.9,0.9,0.9,1}}),
						ui.build(Text{.value = &sliderText, .color = {0.9,0.9,0.9,1}}),
						ui.build(Row{
							.xalign = XAlign::Center,
							.packing = Packing::Tight,
							.children = {
								ui.build(TextInputF64{.value = &mainSliderMin, .size = Vec2{45,25}}),
								ui.build(SliderF64{.value = &sliderValue, .size = Vec2{120,30}, .min = &mainSliderMin, .max = &mainSliderMax}),
								ui.build(TextInputF64{.value = &mainSliderMax, .size = Vec2{45,25}}),
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
										ui.build(Text{.value = &threeSlidersMainMinText, .fontSize = Vec2{7,14}, .color = Vec4{1,1,1,1}}),
										ui.build(SliderF64{.value = &threeSlidersMainMin, .size = Vec2{25,100}, .min = 5.0, .max = 50.0, .bVertical = true}),
									}
								}),
								ui.build(Column{
									.xalign = XAlign::Center,
									.children = {
										ui.build(Text{.value = &threeSlidersMainMaxText, .fontSize = Vec2{7,14}, .color = Vec4{1,1,1,1}}),
										ui.build(SliderF64{.value = &threeSlidersMainMax, .size = Vec2{25,100}, .min = 5.0, .max = 50.0, .bVertical = true}),
									}
								}),
								ui.build(Column{
									.xalign = XAlign::Center,
									.children = {
										ui.build(Text{.value = &threeSlidersMainValueText, .fontSize = Vec2{7,14}, .color = Vec4{1,1,1,1}}),
										ui.build(SliderF64{
											.value = &threeSlidersMainValue,
											.size = Vec2{25,100},
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
					// Footer:
					ui.build(Box{ 
						.bFillSpace = true, 
						.color = DEFAULT_STYLE.fill1,
						.xalign = XAlign::Right, 
						.yalign = YAlign::Center, 
						.child = ui.build(Box{.minsize = Vec2{20,20}, .color = DEFAULT_STYLE.accent0, .bDragable = true, .onDrag = OnDrag::Resize })
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
		initialize("GUI Test", 1000, 1000);
		handle = makeUI();

		setMaxFPS(200);
		ui.printMemoryUtalisation();
	}

	virtual void update(float deltaTime) override final
	{
		threeSlidersMainMinText = std::string("min:") + std::to_string(threeSlidersMainMin);
		threeSlidersMainMaxText = std::string("max:") + std::to_string(threeSlidersMainMax);
		threeSlidersMainValueText = std::string("size:") + std::to_string(threeSlidersMainValue);
		deltaTimeText = genDeltaTimeStr(); 
		sliderText = std::string("max fps: ") + std::to_string(sliderValue);
		gridSize.x = threeSlidersMainValue;
		gridSize.y= threeSlidersMainValue;
		setMaxFPS(std::max(5.0, sliderValue));
		ui.draw(renderer, mainWindow, deltaTime);
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
		if (mainWindow.keyPressed(Key::NP_ADD)) { ui.globalScaling *= 1.0f + deltaTime; }
		if (mainWindow.keyPressed(Key::NP_SUBTRACT)) { ui.globalScaling /= 1.0f + deltaTime; }
	}

	virtual void destroy() override final
	{
	}

private:
	gui::Manager ui;
	gui::Manager::RootHandle handle;
	bool bCheckbox{ false };
	std::string text;
	std::string deltaTimeText;
	Vec2 gridSize{ 25,25 };

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
	TextureManager tmanager;
};
