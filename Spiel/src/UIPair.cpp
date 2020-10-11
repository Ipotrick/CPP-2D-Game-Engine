#pragma once

#include "UIPair.hpp"

void UIPair::draw(std::vector<Drawable>& buffer, UIContext context)
{
	assert(first);
	assert(second);
	context.increaseDrawPrio();

	// shrink context to the elements size and anchor
	context = anchor.shrinkContextToMe(this->size, context);

	first->draw(buffer, context);
	Vec2 positionFirst = first->anchor.getOffset(first->getSize() * context.scale, context);
	Vec2 sizeFirst = first->getSize() * context.scale;

	if (bHorizonal) {
		context.ulCorner.x = positionFirst.x + sizeFirst.x * 0.5f;
		second->draw(buffer, context);
	}
	else {
		context.ulCorner.y = positionFirst.y - sizeFirst.y * 0.5f;
		second->draw(buffer, context);
	}
}