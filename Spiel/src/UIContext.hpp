#pragma once

#include "RenderTypes.hpp"

struct UIContext {
	Vec2 ulCorner{ 0.0f, 0.0f };
	Vec2 drCorner{ 0.0f, 0.0f };
	float scale{ 1.0f };
	float drawingPrio{ 1.0f };
	static constexpr float incrementSize{ 0.0001f };
	RenderSpace drawMode{ RenderSpace::PixelSpace };

	void increaseDrawPrio()
	{
		drawingPrio += incrementSize;
	}

	void debugDraw(std::vector<Drawable>& buffer)
	{
		Vec2 position = (ulCorner + drCorner) * 0.5f;
		Vec2 size = abs(ulCorner - drCorner);
		buffer.push_back(Drawable(0, position, drawingPrio + incrementSize, size, Vec4(1, 0, 0, 0.1f), Form::Rectangle, RotaVec2(0), drawMode));
	}
};