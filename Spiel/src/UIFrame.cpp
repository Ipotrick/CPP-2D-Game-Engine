#include "UIFrame.hpp"
#include "DrawFrame.hpp"

void UIFrame::draw(std::vector<Drawable>& buffer, UIContext context)
{
	context.layer = layer;
	context.scale *=	this->scale;
	context.drawMode =	this->drawMode;
	Vec2 size =			this->size * context.scale;
	Vec2 position =		this->anchor.getOffset(size, context);
	Vec2 borders =		this->borders * context.scale;
	drawFrame(buffer, context, position, size, borders, borderColor, fillColor);
	++context.recursionDepth;
	drawChildren(buffer, context, position, size, borders);

	lastDrawArea = anchor.shrinkContextToMe(this->size, context);
}

void UIFrame::drawChildren(std::vector<Drawable>& buffer, UIContext context, const Vec2 position, const Vec2 size, const Vec2 borders)
{
	if (hasChild()) {
		context = anchor.shrinkContextToMe(this->size, context);
		context.cutOffBorder(borders);
		applyPadding(context);
		getChild()->draw(buffer, context);

		if (bAutoLength) {
			this->size.y = getChild()->getSize().y + getYPadding() + this->borders.y * 2.0f;
		}
	}
}
