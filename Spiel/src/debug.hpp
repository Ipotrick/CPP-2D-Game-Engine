#pragma once

#include <array>

#include "BaseTypes.hpp"
#include "RenderTypes.hpp"
#include "Vec2.hpp"

std::array<Drawable, 3> makeArrow(Vec2 vec, Vec2 origin) {
	float const width = 0.03f;
	Vec4 const color = Vec4(1, 0, 0, 1);
	float len = length(vec);
	float angle = getRotation(vec);
	RotaVec2 rotaVec = RotaVec2(angle);
	RotaVec2 rotaVecTip = RotaVec2(angle + 45);
	auto centerPos = origin + vec * 0.5f;
	return {
		Drawable(0, centerPos, 0.95f, Vec2(len, width), color, Form::Rectangle, rotaVec),
		Drawable(0, origin, 0.95f, Vec2(width, width) , color, Form::Circle, rotaVec),
		Drawable(0, origin + vec, 0.95f, Vec2(width, width) / sqrt(2), color, Form::Rectangle, rotaVecTip)
	};
}