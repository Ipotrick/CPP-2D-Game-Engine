#pragma once

#include "UIElement.hpp"
#include "UIClickable.hpp"

class UIButton : public UIClickable {
public:
	virtual void draw(std::vector<Drawable>& buffer, UIContext context) override
	{
		// cap borders so that they never escape the button
		border = std::min(border, this->size.x * 0.5f);
		border = std::min(border, this->size.y * 0.5f);

		Vec4 innerColor = bPressed ? innerPressedColor : innerReleasedColor;

		Vec2 size = this->size * context.scale;
		Vec2 position = anchor.getOffset(size, context);
		float border = this->border * context.scale;

		buffer.push_back(Drawable(0, position, context.drawingPrio, size, borderColor, Form::Rectangle, RotaVec2(0), context.drawMode));
		context.increaseDrawPrio();
		buffer.push_back(Drawable(0, position, context.drawingPrio, size - Vec2(border, border), innerColor, Form::Rectangle, RotaVec2(0), context.drawMode));

		lastDrawArea = context;
		lastDrawArea.ulCorner = position - Vec2(1, -1) * size * 0.5f;
		lastDrawArea.drCorner = position - Vec2(-1, 1) * size * 0.5f;
	}

	float border{ 2.0f };
	Vec4 borderColor{ 0.0f, 0.0f, 0.0f, 1.0f };
	Vec4 innerReleasedColor{ 0.8f, 0.8f, 0.8f, 1.0f };
	Vec4 innerPressedColor{ 0.2f, 0.2f, 0.8f, 1.0f };
private:
};