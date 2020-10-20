#pragma once

#include "UISingleParent.hpp"
#include "UIFocusable.hpp"

class UIFrame : public UIFocusable, public UISingleParent {
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
	//void setTextureAtlas(const SmallTextureRef& texRef) { texAtlas = texRef; }
	//void setTextureAtlas(SmallTextureRef&& texRef) { texAtlas = texRef; }
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

private:
	//void drawCorners(std::vector<Drawable>& buffer, UIContext context, const Vec2 position, const Vec2 size, const Vec2 borders);
	//void drawBorders(std::vector<Drawable>& buffer, UIContext context, const Vec2 position, const Vec2 size, const Vec2 borders);
	//void drawFill(std::vector<Drawable>& buffer, UIContext context, const Vec2 position, const Vec2 size, const Vec2 borders);
	void drawChildren(std::vector<Drawable>& buffer, UIContext context, const Vec2 position, const Vec2 size, const Vec2 borders);

	Vec2 borders{ standartBorder };
	bool bBorderScalable{ true };
	RenderSpace drawMode{ RenderSpace::PixelSpace };
	//SmallTextureRef texAtlas;
	float scale{ 1.0f };
};