#pragma once

#include <functional>
#include <optional>

#include "World.hpp"

struct UIContext {
	Vec2 ulCorner{ 0.0f, 0.0f };
	Vec2 drCorner{ 0.0f, 0.0f };
	float scale{ 1.0f };
	int depth{ 0 };
	DrawMode drawMode{ DrawMode::PixelSpace };
};

class UIElement {
public:
	virtual void draw(std::vector<Drawable>& buffer, UIContext context) = 0;
	void destroy() { 
		b_destroyMark = true;
		if (hasChild()) {
			getChild()->destroy();
		}
	}
	bool isDestroyed() const { return b_destroyMark; }

	void giveChild(UIElement* child)
	{
		childElement = child;
	}
	const UIElement* getChild() const
	{
		return childElement;
	}
	UIElement* getChild()
	{
		return childElement;
	}
	bool hasChild() const { return childElement != nullptr; }
protected:
	std::pair<Vec2, Vec2> lastDrawnArea;	// this is used to check for mouse intersections 
private:
	bool b_destroyMark{ false };
	UIElement* childElement{ nullptr };
};