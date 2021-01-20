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
			.sizeing = Sizeing().absX(100).absY(100),
			.placeing = Placeing().absDistLeft(100).absDistTop(100),
			.color = Vec4{ 1, 0, 0.66, 1 },
			.child = ui.build( 
				Column{
				.xalignment = XAlign::Center,
				.yalignment = YAlign::Center,
				.listing = Listing::Spread,
				.children = {
					ui.build(Box{.minsize = {10,10}, .color = {1,1,1,1}}),
					ui.build(Box{.minsize = {15,15}, .color = {1,1,1,1}}),
					ui.build( 
						gui::GUIButton{
						.size = {25,25},
						.onPress = [&](GUIButton& self) { std::cout << "Button Press\n"; },
						.onRelease = [&](GUIButton& self) { std::cout << "Button Release\n"; },
					}),
				},
			})
		});
	}

	virtual void create() override 
	{
		initialize("GUI Test", 1000, 1000);
		handle = makeUI();
	}

	virtual void update(float deltaTime) override 
	{
		if (in.keyJustPressed(Key::A)) {
			if (ui.isHandleValid(handle)) {
				ui.destroy(handle);
			}
		}
		if (in.keyJustPressed(Key::S)) {
			if (!ui.isHandleValid(handle)) {
				handle = makeUI();
			}
		}
		if (in.keyJustPressed(Key::D)) {
			std::cout << "there currently exist " << ui.size() << " ui elements " << std::endl;
		}

		gui::DrawContext context;
		context.topleft = Vec2{ 0, static_cast<float>(window.getSize().second) };
		context.bottomright = Vec2{ static_cast<float>(window.getSize().first), 0 };
		context.scale = uiscale;
		ui.draw(context, renderer, in);
	}

	virtual void destroy() override
	{
	}

private:
	gui::RootElementHandle handle;
	gui::Manager ui;
	float uiscale{ 1.0f };
};
