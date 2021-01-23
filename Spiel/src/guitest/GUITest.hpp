#pragma once

#include "../engine/EngineCore.hpp"
#include "../engine/gui/GUIManager.hpp"

class GUIApp : public EngineCore {
public:

	gui::RootElementHandle makeUI()
	{
		using namespace gui;
		
		return ui.build( 
		Root{
		.sizeing	= { 200, 600 },
		.placeing	= Placing().relDistLeft(0.5).relDistBottom(0.5f),
		.child		= ui.build( 
			FillBox{
			.color		= Vec4::From256( 100, 100, 150, 255 ),
			.xalignment = XAlign::Center,
			.yalignment = YAlign::Center,
			.child		= ui.build(
				Column{
				.children	= {
					ui.build(
						FillBox{
						.xalignment = XAlign::Center,
						.yalignment = YAlign::Center,
						.child		= ui.build(StaticText{.str = "Test UI Element"}),
					}),
					ui.build(
						gui::Button{
						.size	= { 25, 25 },
						.onRelease = [&](Button& self) { std::cout << "button1\n"; },
					}),
					ui.build(
						gui::Button{
						.size	= { 25, 25 },
						.onRelease = [&](Button& self) { std::cout << "button2\n"; },
					}),
					ui.build(
						Box{
						.size	= { 190, 30 }, 
						.child	= ui.build(DragField{})
					}),
					ui.build(
						TextInput{
							.size = {190, 50},
							.onStore = [](std::string str) { std::cout << str << std::endl; },
							.bClearOnEnter = true,
					}),
					}
				}),
			}),
		});
	}

	virtual void create() override 
	{
		initialize("GUI Test", 1000, 1000);
		handle = makeUI();
		handle2 = makeUI();
		setMaxFPS(144);
	}

	virtual void update(float deltaTime) override
	{
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

	virtual void destroy() override
	{
	}

private:
	gui::RootElementHandle handle;
	gui::RootElementHandle handle2;
	gui::Manager ui;
	float uiscale{ 1.0f };
};
