#include "UIButton.hpp"

inline void UIButton::draw(std::vector<Sprite>& buffer, UIContext context)
{
	// cap borders so that they never escape the button
	border = std::min(border, this->size.x * 0.5f);
	border = std::min(border, this->size.y * 0.5f);

	Vec4 innerColor = bPressed ? innerPressedColor : innerReleasedColor;

	Vec2 size = this->size * context.scale;
	Vec2 position = anchor.getOffset(size, context);
	float border = this->border * context.scale;

	buffer.push_back(makeSprite(0, position, 0, size, borderColor, Form::Rectangle, RotaVec2(0), context.drawMode));
	++context.recursionDepth;
	buffer.push_back(makeSprite(0, position, 0, size - Vec2(border, border) * 2.0f, innerColor, Form::Rectangle, RotaVec2(0), context.drawMode));

	lastDrawArea = context;
	lastDrawArea.ulCorner = position - Vec2(1, -1) * size * 0.5f;
	lastDrawArea.drCorner = position - Vec2(-1, 1) * size * 0.5f;
}
