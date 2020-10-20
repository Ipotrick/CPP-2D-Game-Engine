#pragma once

#include "UIMultiParent.hpp"

template<std::size_t CAPACITY>
class UIList : public UIMultiParent<CAPACITY> {
public:

	virtual void draw(std::vector<Drawable>& buffer, UIContext context) override
	{
		context = this->anchor.shrinkContextToMe(this->size, context);
		float offset{ 0.0f };

		if (bHorizontal) {
			if (bSpacingUniform) {
				float wholeSize = this->size.x;
				float childAccSize{ 0.0f };
				for (UIElement* child : this->children) {
					childAccSize += child->getSize().x;
				}
				this->spacing = (wholeSize - childAccSize) / ((float)this->children.size() - 1.0f);
			}
			for (auto& child : this->children) {
				UIContext c = context;
				c.ulCorner = context.ulCorner + Vec2(offset, 0.0f);
				c.drCorner.y = context.drCorner.y;
				c.drCorner.x = c.ulCorner.x + child->getSize().x * context.scale;
				//c.debugDraw(buffer);
				child->draw(buffer, c);

				offset += (child->getSize().x + spacing) * context.scale;
			}
		}
		else {
			if (bSpacingUniform) {
				float wholeSize = this->size.y;
				float childAccSize{ 0.0f };
				for (UIElement* child : this->children) {
					childAccSize += child->getSize().y;
				}
				this->spacing = (wholeSize - childAccSize) / ((float)this->children.size() - 1.0f);
			}
			for (auto& child : this->children) {
				UIContext c = context;
				c.ulCorner = context.ulCorner - Vec2(0.0f, offset);
				c.drCorner.x = context.drCorner.x;
				c.drCorner.y = c.ulCorner.y - child->getSize().y * context.scale;
				//c.debugDraw(buffer);
				child->draw(buffer, c);

				offset += (child->getSize().y + spacing) * context.scale;
			}
		}
	}

	void setSpacing(const float spacing) { this->spacing = spacing; }
	float getSpacing() const { return spacing; }

	void setSpacingUniform(const bool b = true)
	{
		this->bSpacingUniform = b;
	}
	bool isSpacingUniform() const
	{
		return this->bSpacingUniform;
	}

	void setHorizontal(const bool b) { this->bHorizontal = b; }
	bool isHorizontal() const { return bHorizontal; }
private:
	float spacing{ 0.0f };
	bool bSpacingUniform{ false };
	bool bHorizontal{ false };
};

using UIList8 = UIList<8>;
using UIList16 = UIList<16>;
using UIList32 = UIList<32>;
using UIList64 = UIList<32>;