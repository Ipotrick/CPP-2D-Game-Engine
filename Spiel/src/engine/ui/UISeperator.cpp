#include "UISeperator.hpp"

void UISeperator::draw(std::vector<Sprite>& buffer, UIContext context)
{
	++context.recursionDepth;

	if (bAutoWidth) {
		width = (bHorizontal ? (context.drCorner.x - context.ulCorner.x) : (context.ulCorner.y - context.drCorner.y)) / context.scale;
	}

	this->size = bHorizontal ? Vec2{ width, thiccnes } : Vec2{ thiccnes, width };

	auto pos = anchor.getOffset(this->size * context.scale, context);

	buffer.push_back(makeSprite(0, pos, 0, this->size * context.scale, color, Form::Rectangle, RotaVec2(0), context.drawMode));
}
