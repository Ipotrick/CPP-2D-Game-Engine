#include "UIAnchor.hpp"

float UIAnchor::getXOffset(const float size, const UIContext& context) const
{
	float scale = bContextScalable ? context.scale : 1.0f;
	switch (xMode) {
	case X::DirectPosition:
		return context.ulCorner.x + xAnchor * scale;
	case X::LeftAbsoluteDist:
		return context.ulCorner.x + xAnchor * scale + size * 0.5f;
	case X::RightAbsoluteDist:
		return context.drCorner.x - xAnchor * scale - size * 0.5f;
	case X::LeftRelativeDist:
		return context.ulCorner.x * (xAnchor)+context.drCorner.x * (1.0f - xAnchor);
	case X::RightRelativeDist:
		return context.ulCorner.x * (1.0f - xAnchor) + context.drCorner.x * (xAnchor);
	default:
		assert(false);
		return 0.0f;
	}
}

float UIAnchor::getYOffset(const float size, const UIContext& context) const
{
	float scale = bContextScalable ? context.scale : 1.0f;
	switch (yMode) {
	case Y::DirectPosition:
		return context.drCorner.y + yAnchor * scale;
	case Y::TopAbsoluteDist:
		return context.ulCorner.y - yAnchor * scale - size * 0.5f;
	case Y::BottomAbsoluteDist:
		return context.drCorner.y + yAnchor * scale + size * 0.5f;
	case Y::TopRelativeDist:
		return context.ulCorner.y * (1.0f - yAnchor) + context.drCorner.y * (yAnchor);
	case Y::BottomRelativeDist:
		return context.ulCorner.y * (yAnchor)+context.drCorner.y * (1.0f - yAnchor);
	default:
		assert(false);
		return 0.0f;
	}
}

UIContext UIAnchor::shrinkContextToMe(const Vec2 size, UIContext context) const
{
	Vec2 position = getOffset(size * context.scale, context);
	context.ulCorner.x = position.x - size.x * 0.5f * context.scale;
	context.ulCorner.y = position.y + size.y * 0.5f * context.scale;
	context.drCorner.x = position.x + size.x * 0.5f * context.scale;
	context.drCorner.y = position.y - size.y * 0.5f * context.scale;
	return context;
}