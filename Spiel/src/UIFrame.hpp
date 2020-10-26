#pragma once

#include "UISingleParent.hpp"
#include "UIFocusable.hpp"
#include "UIPaddingBase.hpp"

class UIFrame : public UIFocusable, public UISingleParent, public UIPaddingBase {
	friend class UIManager;
public:
	UIFrame()
	{ }

	virtual void enable() override
	{
		UIFocusable::enable();
		if (hasChild()) {
			enableChild();
		}
	}
	
	virtual void destroy() override
	{
		UIFocusable::destroy();
		if (hasChild()) {
			destroyChild();
		}
	}

	virtual void disable() override
	{
		UIFocusable::disable();
		if (hasChild()) {
			disableChild();
		}
	}

	virtual void postUpdate() override
	{
		if (hasChild()) {
			getChild()->postUpdate();
		}
	}

	UIFrame(UIAnchor anchor, Vec2 size)
	{ 
		this->size = size;
		this->anchor = anchor;
	}

	void setDrawMode(const RenderSpace drawMode) { this->drawMode = drawMode; }
	RenderSpace getDrawMode() const { return drawMode; }
	void setBorders(float width)
	{
		this->borders.x = width;
		this->borders.y = width;
	}
	void setBorders(float xWidth, float yWidth)
	{
		this->borders.x = xWidth;
		this->borders.y = yWidth;
	}
	Vec2 getBorders() const
	{
		return borders;
	}

	virtual void draw(std::vector<Drawable>& buffer, UIContext context) override;

	void setBorderScalable(const bool b) { this->bBorderScalable = b; }
	bool isBorderScalable() const { return this->bBorderScalable; }

	void setScale(const float f)
	{
		this->scale = f;
	}
	float getScale() const
	{
		return scale;
	}

	virtual void onEnter() override
	{
		onEnterFn(this);
	}

	virtual void onHover() override
	{
		onHoverFn(this);
	}

	virtual void onLeave() override
	{
		onLeaveFn(this);
	}
	
	void setEnterFn(std::function<void(UIFocusable*)> fn)
	{
		this->onEnterFn = fn;
	}
	
	void setHoverFn(std::function<void(UIFocusable*)> fn)
	{
		this->onHoverFn = fn;
	}
	
	void setLeaveFn(std::function<void(UIFocusable*)> fn)
	{
		this->onLeaveFn = fn;
	}

	virtual void setSize(const Vec2 size) override
	{
		this->size = size;
		this->bAutoLength = false;
	}

	void setLength(const float l)
	{
		this->size.y = l;
		this->bAutoLength = false;
	}

	void setWidth(const float w)
	{
		this->size.x = w;
	}

	Vec4 fillColor{ 0,0,0,1 };
	Vec4 borderColor{ 1,1,1,1 };
private:
	void drawChildren(std::vector<Drawable>& buffer, UIContext context, const Vec2 position, const Vec2 size, const Vec2 borders);

	std::function<void(UIFocusable*)> onEnterFn{ [](UIFocusable* me) {} };
	std::function<void(UIFocusable*)> onHoverFn{ [](UIFocusable* me) {} };
	std::function<void(UIFocusable*)> onLeaveFn{ [](UIFocusable* me) {} };
	Vec2 borders{ standartBorder };
	float scale{ 1.0f };
	RenderSpace drawMode{ RenderSpace::PixelSpace };
	bool bBorderScalable{ true };
	bool bAutoLength{ true };
};