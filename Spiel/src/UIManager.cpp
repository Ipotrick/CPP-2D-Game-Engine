#include "UIManager.hpp"

UIFrame& UIManager::createFrame(std::string_view name, UIFrame&& frame)
{
	frames[name] = frame;
	return frames[name];
}

UIElement& UIManager::createElement(std::string_view name, UIElement&& element)
{
	elements[name] = element;
	return elements[name];
}

void UIManager::destroyFrame(std::string_view name)
{
	frames.erase(name);
}

void UIManager::destroyElement(std::string_view name)
{
	elements.erase(name);
}

void UIManager::update()
{
	framesBuffer.clear();
	elementBuffer.clear();

	for (auto [ent, h, b] : world.entityComponentView<Health, Base>()) {
		UIFrame f;
		f.size = Vec2(0.5, 0.15);
		f.position = b.position + Vec2(0, 0.2);
		f.ent = ent;
		f.script = [](UIFrame& f, World& w) -> Drawable {
			float fill = w.getComp<Health>(f.ent).curHealth / w.getComp<Health>(f.ent).maxHealth;

			return Drawable(0, f.position + Vec2(fill / 2, 0), 1.0f, f.size + Vec2(fill, 0), Vec4(1, 0, 0, 1), Form::Rectangle, RotaVec2(0));
		};
		framesBuffer.push_back(f);
	}
}

void UIManager::submitUI()
{
	for (auto [name, frame] : frames) {
		if (frame.script.has_value()) {
			auto d = frame.script.value()(frame, world);
			renderer.submit(d);
		}
		else {
			auto d = Drawable(0, frame.position, 1.0f, frame.size, Vec4(1, 1, 1, 1), Form::Rectangle, RotaVec2(0), DrawMode::PixelSpace);
			renderer.submit(d);
		}
	}
	for (auto frame : framesBuffer) {
		if (frame.script.has_value()) {
			auto d = frame.script.value()(frame, world);
			renderer.submit(d);
		}
		else {
			auto d = Drawable(0, frame.position, 1.0f, frame.size, Vec4(1, 1, 1, 1), Form::Rectangle, RotaVec2(0), DrawMode::PixelSpace);
			renderer.submit(d);
		}
	}

	//for (auto [playerEnt, playerComp, base, move] : world.entityComponentView<Player, Base, Movement>()) {
	//	float b = move.velocity.length() * 20;
	//
	//	auto d = Drawable(0, Vec2(b/2,50), 1.0f, Vec2(b,100), Vec4(1, 1, 1, 1), Form::Rectangle, RotaVec2(0), DrawMode::PixelSpace);
	//	renderer.submit(d);
	//}
}