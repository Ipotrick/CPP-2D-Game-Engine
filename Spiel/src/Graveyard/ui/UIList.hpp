#pragma once

#include "UIMultiParent.hpp"
#include "UIPaddingBase.hpp"

enum class SizeMode {
	Manual,
	FillUp,
	FillMin
};

template<std::size_t CAPACITY>
class UIList : public UIMultiParent<CAPACITY>, public UIPaddingBase {
public:

	virtual void draw(std::vector<Sprite>& buffer, UIContext context) override
	{
		UIContext const origContext = context;
		context = this->anchor.shrinkContextToMe(this->size, context);
		applyPadding(context);
		float offset{ 0.0f };
		float maxElementWidth{ 0.0f };	// width of biggest element in list, used for autosize

		if (bHorizontal) {
			if (bSpacingUniform) {
				float wholeSize = this->size.x;
				float childAccSize{ 0.0f };
				for (UIElement* child : this->children) {
					childAccSize += child->getSize().x;
				}
				this->spacing = (wholeSize - childAccSize) / ((float)this->children.size());

				offset += this->spacing / 2.0f;	// half a spacing in the beginning and half a spacing in the end
			}
			for (auto& child : this->children) {
				UIContext c = context;
				c.cutOffLeft(offset);
				c.cutOffRight(c.getScaledSize().x - child->getSize().x * context.scale);
				child->draw(buffer, c);

				offset += (child->getSize().x + this->spacing) * context.scale;
				maxElementWidth = std::max(maxElementWidth, child->getSize().y);
			}

			if (offset > 0) {
				offset -= spacing * context.scale;
			}
			auto minLength = offset / context.scale;// +getXPadding();
			if (sizeModeX == SizeMode::FillMin) {
				this->size.x = minLength;
			}
			else if (sizeModeX == SizeMode::FillUp) {
				this->size.x = origContext.getUnscaledSize().x;
			}
			if (sizeModeY == SizeMode::FillMin) {
				this->size.y = maxElementWidth;
			}
			else if (sizeModeY == SizeMode::FillUp) {
				this->size.y = origContext.getUnscaledSize().y;
			}
		}
		else {
			if (bSpacingUniform) {
				float wholeSize = this->size.y;
				float childAccSize{ 0.0f };
				for (UIElement* child : this->children) {
					childAccSize += child->getSize().y;
				}
				this->spacing = (wholeSize - childAccSize) / ((float)this->children.size());

				offset += this->spacing / 2.0f;	// half a spacing in the beginning and half a spacing in the end
			}

			for (auto& child : this->children) {
				UIContext c = context;
				c.cutOffTop(offset);
				c.cutOffBottom(c.getScaledSize().y - child->getSize().y * context.scale);
				child->draw(buffer, c);

				offset += (child->getSize().y + spacing) * context.scale;
				maxElementWidth = std::max(maxElementWidth, child->getSize().x);
			}

			if (offset > 0) {
				offset -= spacing * context.scale;
			}
			auto minLength = offset / context.scale + getYPadding();
			if (sizeModeY == SizeMode::FillMin) {
				this->size.y = minLength;
			}

			if (sizeModeX == SizeMode::FillMin) {
				this->size.x = minLength;
			}
			else if (sizeModeY == SizeMode::FillUp) {
				this->size.y = origContext.getUnscaledSize().y;
			}
			if (sizeModeX == SizeMode::FillMin) {
				this->size.x = maxElementWidth;
			}
			else if (sizeModeX == SizeMode::FillUp) {
				this->size.x = origContext.getUnscaledSize().x;
			}
		}
	}

	void setSpacing(const float spacing) { this->spacing = spacing; }
	float getSpacing() const { return spacing; }

	void setSpacingUniform(const bool b = true) { this->bSpacingUniform = b; }
	bool isSpacingUniform() const { return this->bSpacingUniform; }

	virtual void setSize(const Vec2 size) override
	{
		this->size = size;
		sizeModeX = SizeMode::Manual;
		sizeModeY = SizeMode::Manual;
	}

	void setHorizontal(const bool b = true) { this->bHorizontal = b; }
	bool isHorizontal() const { return bHorizontal; }

	void setSizeModeX(SizeMode m) { this->sizeModeX = m; }
	void setSizeModeY(SizeMode m) { this->sizeModeY = m; }
private:
	float spacing{ 0.0f };
	bool bSpacingUniform{ false };
	bool bHorizontal{ false };
	SizeMode sizeModeX{ SizeMode::FillUp };
	SizeMode sizeModeY{ SizeMode::FillUp };
};

using UIList8 = UIList<8>;
using UIList16 = UIList<16>;
using UIList32 = UIList<32>;
using UIList64 = UIList<32>;