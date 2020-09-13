#pragma once

#include <functional>
#include <optional>

#include "World.hpp"

struct UIFrame {
	Entity ent = EntityManager::INVALID_ENTITY;
	Vec2 position;
	Vec2 size;
	TextureRef frameAtlas;
	std::optional<std::function<Drawable(UIFrame&, World& world)>> script = {};
};

class UIElement {
public:
private:
};