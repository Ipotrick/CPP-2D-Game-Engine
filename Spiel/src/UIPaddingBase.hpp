#pragma once

#include "UIContext.hpp"

class UIPaddingBase {
public:
	void setTopPadding(const float f) { this->topPadding = f; }
	float getTopPadding() const { return topPadding; }
	void setBottomPadding(const float f) { this->bottomPadding = f; }
	float getBottomPadding() const { return bottomPadding; }
	void setLeftPadding(const float f) { this->leftPadding = f; }
	float getLeftPadding() const { return leftPadding; }
	void setRightPadding(const float f) { this->rightPadding = f; }
	float getRightPadding() const { return rightPadding; }
	void setPadding(const Vec2 v)
	{
		this->topPadding = v.y;
		this->bottomPadding = v.y;
		this->leftPadding = v.x;
		this->rightPadding = v.x;
	}
	float getXPadding() const { return leftPadding + rightPadding; }
	float getYPadding() const { return topPadding + bottomPadding; }

	void applyPadding(UIContext& context)
	{
		context.cutOffTop(topPadding * context.scale);
		context.cutOffBottom(bottomPadding * context.scale);
		context.cutOffLeft(leftPadding * context.scale);
		context.cutOffRight(rightPadding * context.scale);
	}
protected:
	float topPadding{ 0.0f };
	float bottomPadding{ 0.0f };
	float leftPadding{ 0.0f };
	float rightPadding{ 0.0f };
};