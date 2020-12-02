#pragma once

#include "UIPair.hpp"

void UIPair::draw(std::vector<Drawable>& buffer, UIContext context)
{
	assert(first);
	assert(second);
	// shrink context to the elements size and anchor
	context = anchor.shrinkContextToMe(this->size, context);

	first->draw(buffer, context);
	Vec2 sizeFirst = first->getSize() * context.scale;
	Vec2 positionFirst = first->anchor.getOffset(sizeFirst, context);

	if (bHorizonal) {
		context.ulCorner.x = positionFirst.x + sizeFirst.x * 0.5f;
		if (bAutoSize) {
			this->size = { first->getSize().x + second->getSize().x, std::max(first->getSize().y, second->getSize().y) };
		}
	}
	else {
		context.ulCorner.y = positionFirst.y - sizeFirst.y * 0.5f;
		if (bAutoSize) {
			this->size = { std::max(first->getSize().x, second->getSize().x), first->getSize().y + second->getSize().y };
		}
	}
	second->draw(buffer, context);
}