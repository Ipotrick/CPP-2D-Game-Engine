#pragma once

#include <vector>

#include "robin_hood.h"

#include "World.hpp"
#include "UIElement.hpp"
#include "Renderer.hpp"

class UIManager {
public:
	UIManager(World& world, Renderer& renderer)
		:world{ world }, renderer{ renderer }
	{}

	UIFrame& createFrame(std::string_view name, UIFrame&& frame = UIFrame());
	UIElement& createElement(std::string_view name, UIElement&& element = UIElement());
	void destroyFrame(std::string_view name);
	void destroyElement(std::string_view name);

	void update();
	void submitUI();
private:
	World& world;
	Renderer& renderer;
	// manual:
	robin_hood::unordered_map<std::string_view, UIFrame> frames;
	robin_hood::unordered_map<std::string_view, UIElement> elements;
	// generated:
	std::vector<UIFrame> framesBuffer;
	std::vector<UIFrame> elementBuffer;
};