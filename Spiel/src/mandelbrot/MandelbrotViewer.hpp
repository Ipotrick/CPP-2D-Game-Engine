#pragma once

#include "EngineConfig.hpp"
#include "../engine/Engine.hpp" 

#include "LayerConstants.hpp"
#include "MandelRenderScript.hpp"

class MandelbrotViewer : public Engine {
public:
	virtual void create() override
	{
		renderer.setLayerCount(LAYER_COUNT);

		renderer.getLayer(LAYER_MANDELBROT).renderMode = RenderSpace::WorldSpace;
		renderer.getLayer(LAYER_MANDELBROT).attachRenderScript(std::make_unique<MandelRenderScript>());

		renderer.getLayer(LAYER_UI).renderMode = RenderSpace::PixelSpace;

		UIFrame frame;
		frame.setWidth(220);
		frame.anchor.setLeftAbsolute(10);
		frame.anchor.setTopAbsolute(10);
		frame.setPadding({ 5,5 });

		UIList8 list;
		list.setSizeModeX(SizeMode::FillMin);
		list.setSizeModeY(SizeMode::FillMin);

		{
			UIText zoomText("", renderer.makeSmallTexRef(TextureInfo("ConsolasAtlas2.png")));
			zoomText.setAutoSize(true);
			zoomText.fontSize = { 10,20 };
			zoomText.setUpdateFn(
				[&](UIElement* e) {
					auto* me = (UIText*)e;
					me->text = std::string("zoom: ") + std::to_string(renderer.getCamera().zoom);
				}
			);
			list.addChild(ui.createAndGet(zoomText));
		}
		{
			UIText coordText("", renderer.makeSmallTexRef(TextureInfo("ConsolasAtlas2.png")));
			coordText.setAutoSize(true);
			coordText.fontSize = { 10,20 };
			coordText.setUpdateFn(
				[&](UIElement* e) {
					auto* me = (UIText*)e;
					me->text = std::string("position: ") +
						"\n  x: " +
						std::to_string(renderer.getCamera().position.x) +
						"\n  y: " +
						std::to_string(renderer.getCamera().position.y);
				}
			);
			list.addChild(ui.createAndGet(coordText));
		}
		{
			UIText fpsText("", renderer.makeSmallTexRef(TextureInfo("ConsolasAtlas2.png")));
			fpsText.setAutoSize(true);
			fpsText.fontSize = { 10,20 };
			fpsText.setUpdateFn(
				[&](UIElement* e) {
					auto* me = (UIText*)e;
					me->text = std::string("fps: ") + std::to_string(1.0 / getDeltaTime(100));
				}
			);
			list.addChild(ui.createAndGet(fpsText));

		}

		frame.addChild(ui.createAndGet(list));
		ui.createFrame(frame);
	}

	virtual void update(float deltaTime) override
	{
		auto& cam = renderer.getCamera();
		const float cameraMoveSpeed = 1.0f;
		if (in.keyPressed(Key::W)) {
			renderer.getCamera().position.y += cameraMoveSpeed * deltaTime * 1/cam.zoom;
		}
		if (in.keyPressed(Key::A)) {
			renderer.getCamera().position.x -= cameraMoveSpeed * deltaTime * 1 / cam.zoom;
		}
		if (in.keyPressed(Key::S)) {
			renderer.getCamera().position.y -= cameraMoveSpeed * deltaTime * 1 / cam.zoom;
		}
		if (in.keyPressed(Key::D)) {
			renderer.getCamera().position.x += cameraMoveSpeed * deltaTime * 1 / cam.zoom;
		}
		if (in.keyPressed(Key::LEFT_SHIFT)) {
			renderer.getCamera().zoom *= 1.0f + deltaTime;
		}
		if (in.keyPressed(Key::LEFT_CONTROL)) {
			renderer.getCamera().zoom *= 1.0f - deltaTime;
		}
	}

	virtual void destroy() override
	{

	}
private:
	Vec2 mouseOffset = { 0,0 };
};