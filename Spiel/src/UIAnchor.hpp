#pragma once

#include "Vec2.hpp"
#include "UIContext.hpp"

class UIAnchor {
public:
	enum class X {
		DirectPosition,
		LeftAbsoluteDist,
		RightAbsoluteDist,
		LeftRelativeDist,
		RightRelativeDist
	};
	enum class Y {
		DirectPosition,
		TopAbsoluteDist,
		BottomAbsoluteDist,
		TopRelativeDist,
		BottomRelativeDist
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
		this->xMode = X::LeftAbsoluteDist;
		this->xAnchor = x;
		return *this;
	}
	UIAnchor& setRightAbsolute(const float x)
	{
		this->xMode = X::RightAbsoluteDist;
		this->xAnchor = x;
		return *this;
	}
	UIAnchor& setLeftRelative(const float x)
	{
		assert(x >= 0.0f && x <= 1.0f);
		this->xMode = X::LeftRelativeDist;
		this->xAnchor = x;
		return *this;
	}
	UIAnchor& setRightRelative(const float x)
	{
		assert(x >= 0.0f && x <= 1.0f);
		this->xMode = X::RightRelativeDist;
		this->xAnchor = x;
		return *this;
	}

	UIAnchor& setTopAbsolute(const float y)
	{
		this->yMode = Y::TopAbsoluteDist;
		this->yAnchor = y;
		return *this;
	}
	UIAnchor& setBottomAbsolute(const float y)
	{
		this->yMode = Y::BottomAbsoluteDist;
		this->yAnchor = y;
		return *this;
	}
	UIAnchor& setTopRelative(const float y)
	{
		assert(y >= 0.0f && y <= 1.0f);
		this->yMode = Y::TopRelativeDist;
		this->yAnchor = y;
		return *this;
	}
	UIAnchor& setBottomRelative(const float y)
	{
		assert(y >= 0.0f && y <= 1.0f);
		this->yMode = Y::BottomRelativeDist;
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
	* sets the position directly from the dl corner of the context as the coordinate center
	*/
	UIAnchor& setXPosition(const float x)
	{
		this->xMode = X::DirectPosition;
		this->xAnchor = x;
	}
	/*
	* sets the position directly from the dl corner of the context as the coordinate center
	*/
	UIAnchor& setYPosition(const float y)
	{
		this->yMode = Y::DirectPosition;
		this->yAnchor = y;
	}
	/*
	* sets the position directly from the dl corner of the context as the coordinate center
	*/
	UIAnchor& setAbsPosition(const Vec2 pos)
	{
		this->xMode = X::DirectPosition;
		this->xAnchor = pos.x;
		this->yMode = Y::DirectPosition;
		this->yAnchor = pos.y;
		return *this;
	}
	Vec2 get() const
	{
		return Vec2{ xAnchor, yAnchor };
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
	UIContext shrinkContextToMe(const Vec2 size, UIContext context) const;

protected:
	float getXOffset(const float size, const UIContext& context) const;
	float getYOffset(const float size, const UIContext& context) const;
	float xAnchor{ 0.0f };
	float yAnchor{ 0.0f };
	X xMode{ X::LeftAbsoluteDist };
	Y yMode{ Y::TopAbsoluteDist };
	bool bContextScalable{ false };
};