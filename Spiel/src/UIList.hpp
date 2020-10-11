#pragma once

#include "UIMultiParent.hpp"

template<std::size_t CAPACITY>
class UIList : public UIMultiParent<CAPACITY> {
public:

	virtual void draw(std::vector<Drawable>& buffer, UIContext context) override
	{
		context.debugDraw(buffer);
		context = this->anchor.shrinkContextToMe(this->size, context);
		context.increaseDrawPrio();
		context.debugDraw(buffer);
		context.increaseDrawPrio();
		float offset{ 0.0f };

		if (bHorizontal) {
			for (auto& child : this->children) {
				UIContext c = context;
				c.ulCorner = context.ulCorner + Vec2(offset, 0.0f);
				c.drCorner.y = context.drCorner.y;
				c.drCorner.x = c.ulCorner.x + child->getSize().x * context.scale;
				c.debugDraw(buffer);
				child->draw(buffer, c);

				offset += (child->getSize().x + spacing) * context.scale;
			}
		}
		else {
			for (auto& child : this->children) {
				UIContext c = context;
				c.ulCorner = context.ulCorner - Vec2(0.0f, offset);
				c.drCorner.x = context.drCorner.x;
				c.drCorner.y = c.ulCorner.y - child->getSize().y * context.scale;
				c.debugDraw(buffer);
				child->draw(buffer, c);

				offset += (child->getSize().y + spacing) * context.scale;
			}
		}
	}

	void setSpacing(const float spacing) { this->spacing = spacing; }
	float getSpacing() const { return spacing; }

	void setHorizontal(const bool b) { this->bHorizontal = b; }
	bool isHorizontal() const { return bHorizontal; }
private:
	float spacing{ 0.0f };
	bool bHorizontal{ false };
};

using UIList8 = UIList<8>;
using UIList16 = UIList<16>;
using UIList32 = UIList<32>;
using UIList64 = UIList<32>;