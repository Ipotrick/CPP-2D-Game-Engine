#include "UICollapsable.hpp"

void UICollapsable::draw(std::vector<Drawable>& buffer, UIContext context)
{
	const float sq2{ sqrtf(2.0f) };
	const float isq2{ 1.0f / sqrtf(2.0f) };

	if (bAutoHeadWidth) {
		this->headSize.x = context.getUnscaledSize().x;
	}
	context = anchor.shrinkContextToMe(this->size, context);

	// draw header:
	Vec2 sHeadSize = this->headSize * context.scale;
	Vec2 sHeadPos = anchor.getOffset(sHeadSize, context);
	Vec2 sBorder = this->border * context.scale;
	drawFrame(buffer, context, sHeadPos, sHeadSize, sBorder, borderColor, fillColor);
	context.increaseDrawPrio(); 
	UIContext headContext = context;
	headContext.cutOffBottom(headContext.getScaledSize().y - sHeadSize.y);
	headContext.cutOffBorder(sBorder);

	// draw Arrow:
	Vec2 arrowAreaSize = Vec2{ sHeadSize.y, sHeadSize.y } - sBorder * 2.0f;
	Vec2 arrowSize = arrowAreaSize *isq2 * arrowScale;
	Vec2 arrowPos = Vec2{ arrowAreaSize.x * 0.5f + sBorder.x + sHeadPos.x - sHeadSize.x * 0.5f, sHeadPos.y };
	buffer.push_back(Drawable(0, arrowPos, headContext.drawingPrio, arrowSize, borderColor, Form::Rectangle, RotaVec2(45), headContext.drawMode));
	headContext.increaseDrawPrio();
	Vec2 helperPos = arrowPos + Vec2{ 0.0f, arrowAreaSize.y * 0.25f } *(bCollapsed ? 1.0f : -1.0f);
	Vec2 helperSize = { arrowAreaSize.x, arrowAreaSize.y * 0.5f };
	buffer.push_back(Drawable(0, helperPos, headContext.drawingPrio, helperSize, fillColor, Form::Rectangle, RotaVec2(0), headContext.drawMode));

	// draw title:
	headContext.cutOffLeft(arrowAreaSize.x);
	textElement.setSize(headContext.getUnscaledSize());
	textElement.draw(buffer, headContext);

	// draw body:
	if (!bCollapsed) {
		auto bodyContext = context;
		bodyContext.ulCorner.y -= sHeadSize.y;	// cut head from context
		if (bAutoBodyLength && hasChild()) {
			this->bodyHeight = getChild()->getSize().y + this->border.y * 2.0f;
		}
		Vec2 bodySize = { sHeadSize.x, bodyHeight * bodyContext.scale };
		Vec2 bodyPos = anchor.getOffset(bodySize, bodyContext);
		drawFrame(buffer, bodyContext, bodyPos, bodySize, sBorder, borderColor, fillColor);
		bodyContext.increaseDrawPrio();

		if (hasChild()) {
			auto childContext = bodyContext;
			childContext.cutOffBorder(sBorder);

			getChild()->draw(buffer, childContext);
		}
	}

	// set size:
	if (!bCollapsed) {
		this->size = this->headSize + Vec2(0, bodyHeight);
	}
	else {
		this->size = this->headSize;
	}
	this->lastDrawArea = anchor.shrinkContextToMe(this->size, context);
}
