#include "UICollapsable.hpp"

void UICollapsable::draw(std::vector<Drawable>& buffer, UIContext context)
{
	// draw header:
	Vec2 headSize = this->headSize * context.scale;
	Vec2 headPos = anchor.getOffset(headSize, context);
	drawFrame(buffer, context, headPos, headSize, border, borderColor, fillColor);
	// draw Title:
	textElement.setSize(headSize - border * 2.0f);
	auto textContext = anchor.shrinkContextToMe(textElement.getSize(), context);
	textContext.increaseDrawPrio();
	textElement.draw(buffer, textContext);
	// draw body:
	// set size:
}
