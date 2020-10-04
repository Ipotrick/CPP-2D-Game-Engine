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
	virtual void destroy() final {
		b_destroyMark = true;
		if (hasChild()) {
			getChild()->destroy();
		}
	}
	virtual bool isDestroyed() const final { return b_destroyMark; }

	virtual void update() final
	{
		if (hasUpdateFn()) {
			fn_update(this);
		}
	}
	virtual bool hasUpdateFn() const final
	{
		return static_cast<bool>(fn_update);
	}
	virtual void setUpdateFn(std::function<void(UIElement*)> fn) final
	{
		fn_update = fn;
	}
	virtual std::function<void(UIElement*)> getUpdateFn() const final
	{
		return fn_update;
	}

	virtual void giveChild(UIElement* child) final
	{
		childElement = child;
	}
	virtual const UIElement* getChild() const final
	{
		return childElement;
	}
	virtual UIElement* getChild() final
	{
		return childElement;
	}
	virtual bool hasChild() const final { return childElement != nullptr; }
protected:
	std::pair<Vec2, Vec2> lastDrawnArea;	// this is used to check for mouse intersections 
private:
	bool b_destroyMark{ false };

	UIElement* childElement{ nullptr };

	std::function<void(UIElement*)> fn_update{ {} };
};