#pragma once

#include "UIMultiParent.hpp"
#include "UIPaddingBase.hpp"

template<std::size_t CAPACITY>
class UIList : public UIMultiParent<CAPACITY>, public UIPaddingBase {
public:

	virtual void draw(std::vector<Drawable>& buffer, UIContext context) override
	{
		if (bHorizontal) {
			if (bAutoWidth) this->size.y = context.getUnscaledSize().y;
		}
		else {
			if (bAutoWidth) this->size.x = context.getUnscaledSize().x;
		}
		context = this->anchor.shrinkContextToMe(this->size, context);
		applyPadding(context);
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
				c.cutOffLeft(offset);
				c.cutOffRight(c.getScaledSize().x - child->getSize().x * context.scale);
				child->draw(buffer, c);

				offset += (child->getSize().x + spacing) * context.scale;
			}
			if (bAutoLength) this->size.x = offset / context.scale + getXPadding();
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
				c.cutOffTop(offset);
				c.cutOffBottom(c.getScaledSize().y - child->getSize().y * context.scale);
				child->draw(buffer, c);

				offset += (child->getSize().y + spacing) * context.scale;
			}
			if (bAutoLength) this->size.y = offset / context.scale + getYPadding();
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

	virtual void setSize(const Vec2 size) override
	{
		this->size = size;
		bAutoLength = false;
		bAutoWidth = false;
	}

	void setHorizontal(const bool b) { this->bHorizontal = b; }
	bool isHorizontal() const { return bHorizontal; }

	void setAutoLength(const bool b = true) { this->bAutoLength = b; }
	bool autoLenth() const { return bAutoLength; }

	void setAutoWidth(const bool b = true) { this->bAutoWidth = b; }
	bool autWidth() const { return bAutoWidth; }
private:
	float spacing{ 0.0f };
	bool bSpacingUniform{ false };
	bool bHorizontal{ false };
	bool bAutoLength{ true };
	bool bAutoWidth{ true };
};

using UIList8 = UIList<8>;
using UIList16 = UIList<16>;
using UIList32 = UIList<32>;
using UIList64 = UIList<32>;