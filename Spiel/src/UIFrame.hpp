#pragma once

#include "UISingleParent.hpp"
#include "UIFocusable.hpp"

class UIFrame : virtual public UISingleParent, virtual public UIFocusable {
	friend class UIManager;
public:
	UIFrame()
	{ }

	virtual void disable() override
	{
		UIFocusable::disable();
		UISingleParent::disable();
	}

	UIFrame(UIAnchor anchor, Vec2 size, SmallTextureRef texAtlas)
		:texAtlas{texAtlas}
	{ 
		this->size = size;
		this->anchor = anchor;
	}

	void setDrawMode(const RenderSpace drawMode) { this->drawMode = drawMode; }
	RenderSpace getDrawMode() const { return drawMode; }
	void setTextureAtlas(const SmallTextureRef texRef) { texAtlas = texRef; }
	void setTextureAtlas(SmallTextureRef&& texRef) { texAtlas = texRef; }
	void setBorders(float width)
	{
		borders.x = width;
		borders.y = width;
	}

	virtual void draw(std::vector<Drawable>& buffer, UIContext context) override;

	void setBorderScalable(const bool b) { this->bBorderScalable = b; }
	bool isBorderScalable() const { return this->bBorderScalable; }

	Vec2 borders;
private:
	void drawCorners(std::vector<Drawable>& buffer, UIContext context, const Vec2 position, const Vec2 size, const Vec2 borders);
	void drawBorders(std::vector<Drawable>& buffer, UIContext context, const Vec2 position, const Vec2 size, const Vec2 borders);
	void drawFill(std::vector<Drawable>& buffer, UIContext context, const Vec2 position, const Vec2 size, const Vec2 borders);
	void drawChildren(std::vector<Drawable>& buffer, UIContext context, const Vec2 position, const Vec2 size, const Vec2 borders);

	bool bBorderScalable{ true };
	RenderSpace drawMode{ RenderSpace::PixelSpace };
	SmallTextureRef texAtlas;
};