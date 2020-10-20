#include "UISeperator.hpp"

void UISeperator::draw(std::vector<Drawable>& buffer, UIContext context)
{
	context.increaseDrawPrio();

	if (bAutoWidth) {
		width = (bHorizontal ? (context.drCorner.x - context.ulCorner.x) : (context.ulCorner.y - context.drCorner.y)) / context.scale;
	}

	this->size = bHorizontal ? Vec2{ width, thiccnes } : Vec2{ thiccnes, width };

	auto pos = anchor.getOffset(this->size * context.scale, context);

	buffer.push_back(Drawable(0, pos, context.drawingPrio, this->size * context.scale, color, Form::Rectangle, RotaVec2(0), context.drawMode));
}
