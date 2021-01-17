#pragma once

#include "../engine/EngineCore.hpp"
#include "../engine/gui/GUIManager.hpp"

class GUIApp : public EngineCore {
public:

	gui::RootElementHandle makeUI()
	{
		using namespace gui;
		return ui.build(Root{
			.sizeing = Sizeing().absX(100).absY(100),
			.placeing = Placeing().absDistLeft(100).absDistTop(100),
			.color = Vec4{ 1, 0, 0.66, 1 },
			.child = ui.build(Column{
				.xalignment = XAlign::Left,
				.yalignment = YAlign::Top,
				.listing = Listing::Uniform,
				.children = {
					ui.build(FillBox{}),
					ui.build(FillBox{}),
					ui.build(Box{.minsize = {35,35}, .color = {0,0,0,1}}),
					ui.build(FillBox{}),
					ui.build(FillBox{}),
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
		ui.draw(context, renderer);
	}

	virtual void destroy() override
	{
	}

private:
	gui::RootElementHandle handle;
	gui::Manager ui;
	float uiscale{ 1.0f };
};
