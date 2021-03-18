#pragma once

#include <vector>
#include <string>

#include "../types/BaseTypes.hpp"
#include "../math/Vec.hpp"
#include "RenderSpace.hpp"

#include "OpenGLAbstraction/OpenGLTexture.hpp"

/**
 * A Sprite is a colored Rectangle or a Circle, that can optinaly be textured.
 */
struct Sprite {
	Vec4 color{ 1.0f, 1.0f, 1.0f, 1.0f };
	Vec3 position;
	/* in radiants 2pi = one rotation*/
	RotaVec2 rotationVec;
	Vec2 scale{ 1.0f, 1.0f };
	TextureHandle texHandle;
	Vec2 texMin{ 0,0 };
	Vec2 texMax{ 1,1 };
	Vec2 clipMin{ -1.0f, -1.0f };
	Vec2 clipMax{ 1.0f, 1.0f };
	float cornerRounding{ 0.0f };
	bool isMSDF{ false };
	RenderSpace drawMode{ RenderSpace::Window };
};

inline static std::ostream& operator<<(std::ostream& os, const Sprite& sprite)
{
	os << "color: " << sprite.color <<
		", position:" << sprite.position <<
		", rotation: " << sprite.rotationVec.toUnitX0() <<
		", scale: " << sprite.scale <<
		", corner rounding: " << sprite.cornerRounding <<
		", isMSDF: " << sprite.isMSDF;
	return os;
}

inline Sprite makeSprite(uint32_t id, Vec2 position, float drawingPrio, Vec2 scale, Vec4 color, Form form, RotaVec2 rotation, RenderSpace drawMode = RenderSpace::Camera, int layer = 0, int layerCount = 0)
{
	auto squishDrawingPrio = [](float f) {
		float s = f < 0.0f ? -1.0f : 1.0f;
		return 0.5f * s * 1.0f - (1.0f / (std::fabs(f) + 1.0f)) + 0.5f;
	};

	return Sprite{
		.color = color,
		.position = {position.x, position.y, f32(layer) / f32(layerCount)},
		.rotationVec = rotation,
		.scale = scale,
		.cornerRounding = (form == Form::Rectangle ? 0.0f : std::min(scale.x,scale.y)),
		.drawMode = drawMode
	};
}