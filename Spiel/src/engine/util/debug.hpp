#pragma once

#include <array>

#include "../types/BaseTypes.hpp"
#include "../rendering/RenderTypes.hpp"
#include "../math/Vec2.hpp"

inline std::array<Sprite, 3> makeArrow(Vec2 vec, Vec2 origin, Vec4 color = { 1, 0, 0, 1 })
{
	Vec2 size = { 0.04f, 0.04f };

	Vec2 end = origin + vec;
	Vec2 middle = origin + 0.5f * vec;

	return {
		makeSprite(0, origin, 1, size, color, Form::Circle, RotaVec2(), RenderSpace::WorldSpace),
		makeSprite(0, end, 1, size, color, Form::Circle, RotaVec2(), RenderSpace::WorldSpace),
		makeSprite(0, middle, 1, {vec.length(), size.y }, color, Form::Rectangle, RotaVec2{ getRotation(vec) }, RenderSpace::WorldSpace),
	};
}