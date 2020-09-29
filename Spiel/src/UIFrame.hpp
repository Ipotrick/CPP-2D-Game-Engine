#pragma once

#include "RenderTypes.hpp"
#include "UIElement.hpp"
#include "UIText.hpp"



class UIFrame : public UIElement {
	friend class UIManager;
public:
	UIFrame()
	{ }

	UIFrame(Vec2 pos, Vec2 size, TextureRef texAtlas)
		:position{pos}, size{size}, texAtlas{texAtlas}
	{ }

	void setPosition(const Vec2 position) { this->position = position; }
	Vec2 getPosition() const { return position; }
	void setSize(const Vec2 size) { this->size = size; }
	Vec2 getSize() const { return size; }
	void setDrawMode(const DrawMode drawMode) { this->drawMode = drawMode; }
	DrawMode getDrawMode() const { return drawMode; }
	void setTextureAtlas(const TextureRef texRef) { texAtlas = texRef; }
	void setTextureAtlas(TextureRef&& texRef) { texAtlas = texRef; }
	void setBorders(float width)
	{
		borders.x = width;
		borders.y = width;
	}
	void setScaling(float scale)
	{
		this->scaling = scale;
	}
	float getScalin() const
	{
		return this->scaling;
	}

	virtual void draw(std::vector<Drawable>& buffer, UIContext context) override;

	Vec2 borders;
private:

	void drawCorners(std::vector<Drawable>& buffer, UIContext context);
	void drawBorders(std::vector<Drawable>& buffer, UIContext context);
	void drawFill(std::vector<Drawable>& buffer, UIContext context);
	void drawChildren(std::vector<Drawable>& buffer, UIContext context);

	Vec2 position	{ 0.0f, 0.0f };
	Vec2 size		{ 0.0f, 0.0f };
	float scaling{ 1.0f };
	DrawMode drawMode{ DrawMode::PixelSpace };
	TextureRef texAtlas;
};