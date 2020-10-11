#pragma once

#include "Vec2.hpp"
#include "UIContext.hpp"

class UIAnchor {
public:
	enum class X {
		LeftAbsolute,
		RightAbsolute,
		LeftRelative,
		RightRelative
	};
	enum class Y {
		TopAbsolute,
		BottomAbsolute,
		TopRelative,
		BottomRelative
	};

	UIAnchor() = default;

	UIAnchor(X xMode, float x, Y yMode, float y)
		:xMode{ xMode }, xAnchor{ x }, yMode{ yMode }, yAnchor{ y }
	{}

	UIAnchor& setContextScalable(const bool scalable) { 
		this->bContextScalable = scalable;
		return *this;
	}
	bool isContextScalable() const { return bContextScalable; }

	UIAnchor& setLeftAbsolute(const float x)
	{
		this->xMode = X::LeftAbsolute;
		this->xAnchor = x;
		return *this;
	}
	UIAnchor& setRightAbsolute(const float x)
	{
		this->xMode = X::RightAbsolute;
		this->xAnchor = x;
		return *this;
	}
	UIAnchor& setLeftRelative(const float x)
	{
		assert(x >= 0.0f && x <= 1.0f);
		this->xMode = X::LeftRelative;
		this->xAnchor = x;
		return *this;
	}
	UIAnchor& setRightRelative(const float x)
	{
		assert(x >= 0.0f && x <= 1.0f);
		this->xMode = X::RightRelative;
		this->xAnchor = x;
		return *this;
	}

	UIAnchor& setTopAbsolute(const float y)
	{
		this->yMode = Y::TopAbsolute;
		this->yAnchor = y;
		return *this;
	}
	UIAnchor& setBottomAbsolute(const float y)
	{
		this->yMode = Y::BottomAbsolute;
		this->yAnchor = y;
		return *this;
	}
	UIAnchor& setTopRelative(const float y)
	{
		assert(y >= 0.0f && y <= 1.0f);
		this->yMode = Y::TopRelative;
		this->yAnchor = y;
		return *this;
	}
	UIAnchor& setBottomRelative(const float y)
	{
		assert(y >= 0.0f && y <= 1.0f);
		this->yMode = Y::BottomRelative;
		this->yAnchor = y;
		return *this;
	}

	UIAnchor& setCenterHorizontal()
	{
		return setLeftRelative(0.5f);
	}

	UIAnchor& setCenterVertical()
	{
		return setTopRelative(0.5f);
	}

	/*
	* This function takes SCALED size of object and the sub context of the anchored element
	* to enable scaling of the absolute distances call setContextScaling(true)
	* returns the center position of the anchored object
	*/
	Vec2 getOffset(const Vec2 scaledSize, const UIContext& context) const 
	{
#ifdef _DEBUG
		// The elements size must be smaller than the size of the context
		// the element must fit into the context
		Vec2 diff = abs(context.ulCorner - context.drCorner) - scaledSize * context.scale;
		assert(diff.x >= 0.0f && diff.y >= 0.0f);
#endif
		return { 
			getXOffset(scaledSize.x, context),
			getYOffset(scaledSize.y, context)
		};
	}

	Vec2 getScaledOffset(const Vec2 unsclaedSize, const UIContext& context) const
	{
		return getOffset(unsclaedSize * context.scale, context);
	}
	X getXMode() const { return xMode; }
	Y getYMode() const { return yMode; }
	float getX() const { return xAnchor; }
	float getY() const { return yAnchor; }

	/*
	* This function takes UNSCALED size of object and the sub context of the anchored element
	* the returnd context is as large as the given SCALED size with the disposition of the
	* anchor
	*/
	UIContext shrinkContextToMe(const Vec2 size, UIContext context)
	{
		Vec2 position = getOffset(size * context.scale, context);
		context.ulCorner.x = position.x - size.x * 0.5f * context.scale;
		context.ulCorner.y = position.y + size.y * 0.5f * context.scale;
		context.drCorner.x = position.x + size.x * 0.5f * context.scale;
		context.drCorner.y = position.y - size.y * 0.5f * context.scale;
		return context;
	}

private:
	float getXOffset(const float size, const UIContext& context) const 
	{
		float scale = bContextScalable ? context.scale : 1.0f;
		switch (xMode) {
		case X::LeftAbsolute:
			return context.ulCorner.x + xAnchor * scale + size * 0.5f;
		case X::RightAbsolute:
			return context.drCorner.x - xAnchor * scale - size * 0.5f;
		case X::LeftRelative:
			return context.ulCorner.x * (xAnchor) + context.drCorner.x * (1.0f - xAnchor);
		case X::RightRelative:
			return context.ulCorner.x * (1.0f - xAnchor) + context.drCorner.x * (xAnchor);
		default:
			return 0.0f;
		}
	}
	float getYOffset(const float size, const UIContext& context) const
	{
		float scale = bContextScalable ? context.scale : 1.0f;
		switch (yMode) {
		case Y::TopAbsolute:
			return context.ulCorner.y - yAnchor * scale - size * 0.5f;
		case Y::BottomAbsolute:
			return context.drCorner.y + yAnchor * scale + size * 0.5f;
		case Y::TopRelative:
			return context.ulCorner.y * (1.0f - yAnchor) + context.drCorner.y * (yAnchor);
		case Y::BottomRelative:
			return context.ulCorner.y * (yAnchor) + context.drCorner.y * (1.0f - yAnchor);
		default:
			return 0.0f;
		}
	}
	bool bContextScalable{ false };
	float xAnchor{ 0.0f };
	float yAnchor{ 0.0f };
	X xMode{ X::LeftAbsolute };
	Y yMode{ Y::TopAbsolute };
};