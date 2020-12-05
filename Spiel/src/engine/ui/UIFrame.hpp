#pragma once

#include "UISingleParent.hpp"
#include "UIFocusable.hpp"
#include "UIPaddingBase.hpp"


class UIFrame : public UIFocusable, public UISingleParent, public UIPaddingBase {
	friend class UIManager;
public:
	struct Parameters {
		// UIElement:
		UIAnchor anchor;
		Vec2 size{ 0.0f, 0.0f };
		std::function<void(UIElement*)> fn_update{ {} };
		std::function<bool(UIElement*)> fn_enableIf{ {} };
		// UIFocusable:
		bool focusable = true;
		// UIPaddingBase:
		std::optional<Vec2> uniPadding{ std::nullopt };
		std::optional<float> padding{ std::nullopt };
		float topPadding{ 0.0f };
		float bottomPadding{ 0.0f };
		float leftPadding{ 0.0f };
		float rightPadding{ 0.0f };
		// UIFrame:
		Vec4 fillColor{ 0,0,0,1 };
		Vec4 borderColor{ 1,1,1,1 };
		int layer{ 1 };
		std::function<bool(UIElement*)> fn_destroyIf{ {} };
		std::function<void(UIFocusable*)> onEnterFn{ [](UIFocusable* me) {} };
		std::function<void(UIFocusable*)> onHoverFn{ [](UIFocusable* me) {} };
		std::function<void(UIFocusable*)> onLeaveFn{ [](UIFocusable* me) {} };
		Vec2 borders{ standartBorder };
		float scale{ 1.0f };
		RenderSpace drawMode{ RenderSpace::PixelSpace };
		bool bBorderScalable{ true };
		bool bAutoLength{ true };
	};

	UIFrame(Parameters param = Parameters())
	{ 
		// UIElement:
		this->size = param.size;
		this->anchor = param.anchor;
		this->fn_update = param.fn_update;
		this->fn_enableIf = param.fn_enableIf;
		// UIFocusable:
		this->bFocusable = param.focusable;
		// UIPaddingBase:
		if (param.padding.has_value()) {
			float padd = param.padding.value();
			this->topPadding = padd;
			this->bottomPadding = padd;
			this->leftPadding = padd;
			this->rightPadding = padd;
		}
		else if (param.uniPadding.has_value()) {
			Vec2 padd = param.uniPadding.value();
			this->setPadding(padd);
		}
		else {
			this->topPadding = param.topPadding;
			this->bottomPadding = param.bottomPadding;
			this->leftPadding = param.leftPadding;
			this->rightPadding = param.rightPadding;
		}
		// UIFrame:
		this->fillColor = param.fillColor;
		this->borderColor = param.borderColor;
		this->layer = param.layer;
		this->fn_destroyIf = param.fn_destroyIf;
		this->onEnterFn = param.onEnterFn;
		this->onHoverFn = param.onHoverFn;
		this->onLeaveFn = param.onLeaveFn;
		this->borders = param.borders;
		this->scale = param.scale;
		this->drawMode = param.drawMode;
		this->bBorderScalable = param.bBorderScalable;
		this->bAutoLength = param.bAutoLength;
	}

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

	void update()
	{
		if (!isDestroyed()) {
			if (hasDestroyIfFn() && fn_destroyIf(this)) {
				destroy();
			}
			else {
				if (hasEnableIfFn()) {
					if (fn_enableIf(this)) {
						enable();
					}
					else {
						disable();
					}
				}
				if (hasUpdateFn()) {
					fn_update(this);
				}
			}
		}
	}

	/*
	* the destroyIf function is called before the update function.
	*/
	void setDestroyIfFn(std::function<bool(UIElement*)> f)
	{
		this->fn_destroyIf = f;
	}
	bool hasDestroyIfFn() const
	{
		return static_cast<bool>(fn_destroyIf);
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

	int layer{ 1 };
private:
	void drawChildren(std::vector<Drawable>& buffer, UIContext context, const Vec2 position, const Vec2 size, const Vec2 borders);

	/**
	* destroyIf function is called before the update function.
	*/
	std::function<bool(UIElement*)> fn_destroyIf{ {} };
	std::function<void(UIFocusable*)> onEnterFn{ [](UIFocusable* me) {} };
	std::function<void(UIFocusable*)> onHoverFn{ [](UIFocusable* me) {} };
	std::function<void(UIFocusable*)> onLeaveFn{ [](UIFocusable* me) {} };
	Vec2 borders{ standartBorder };
	float scale{ 1.0f };
	RenderSpace drawMode{ RenderSpace::PixelSpace };
	bool bBorderScalable{ true };
	bool bAutoLength{ true };
};