#include "UIBar.hpp"

void UIBar::draw(std::vector<Drawable>& buffer, UIContext context)
{
	Vec2 size = this->size * context.scale;
	auto position = this->anchor.getOffset(size, context);

	// background of bar:
	buffer.push_back(Drawable(0, position, context.recursionDepth, size, emptyColor, Form::Rectangle, RotaVec2(0), context.drawMode));
	// bar:
	position.x -= size.x * 0.5f * (1.0f - fill);
	size.x = size.x * fill;
	++context.recursionDepth;
	buffer.push_back(Drawable(0, position, context.recursionDepth, size, fillColor, Form::Rectangle, RotaVec2(0), context.drawMode));
}