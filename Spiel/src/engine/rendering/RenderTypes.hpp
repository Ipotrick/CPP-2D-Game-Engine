#pragma once

#include <vector>
#include <string>
#include <optional>

#include <boost/static_string.hpp>

#include "robin_hood.h"

#include "GL/glew.h"

#include "../types/BaseTypes.hpp"
#include "../math/Vec.hpp"
#include "Camera.hpp"
#include "Window.hpp"
#include "RenderSpace.hpp"

using TextureString = boost::static_string<32>;

using TextureId = int;

struct TextureDiscriptor {
	TextureDiscriptor() = default;

	TextureDiscriptor(const TextureString& name, GLint minFilter = GL_NEAREST, GLint magFilter = GL_NEAREST)
		:name{ name }, minFilter{ minFilter }, magFilter{ magFilter }
	{ }

	bool operator==(const TextureDiscriptor& other) const
	{
		return this->name == other.name 
			&& this->minFilter == other.minFilter
			&& this->magFilter == other.magFilter;
	}

	TextureString name;
	GLint minFilter{ GL_NEAREST };
	GLint magFilter{ GL_NEAREST };
};

struct TextureRef {
	TextureRef(int textureId = -1, Vec2 minPos_ = Vec2{ 0.001f, 0.001f }, Vec2 maxPos_ = Vec2{ 0.999f, 0.999f }) :
		minPos{ minPos_ },
		maxPos{ maxPos_ },
		id{ textureId }
	{
	}

	bool valid() const { return id != -1; }

	Vec2 minPos{ 0.0f, 0.0f };
	Vec2 maxPos{ 1.0f, 1.0f };
	TextureId id{ -1 };
};


/*
*	one can not create a texture ref, you can copy an existing texref or request one from the renderer.
*	one can not change the filename or the settings of a texture ref.
*	If differenct filename or setting is desired, request a new texture ref from the renderer.
*	The min and max pos can be modified.
*/
class BigTextureRef : private TextureRef {
public:
	const TextureString& getFilename() const
	{
		return info.name;
	}
	const TextureDiscriptor& getInfo() const
	{
		return info;
	}
	TextureId getId() const
	{
		return this->id;
	}
	using TextureRef::minPos;
	using TextureRef::maxPos;

	BigTextureRef()
		:info{"default"}, TextureRef{}
	{}

	BigTextureRef(const TextureString& filename, Vec2 minPos = { 0.0f, 0.0f }, Vec2 maxPos = { 1.0f, 1.0f })
		:info{ filename }, TextureRef{ -1, minPos, maxPos }
	{}

	BigTextureRef(TextureDiscriptor info, TextureRef ref)
		:info{ info }, TextureRef{ ref }
	{}

	bool good() const { return id != -1;  }

	TextureRef makeSmall() const
	{
		return TextureRef{ id, minPos, maxPos };
	}
protected:
	friend class TextureRefManager;
	BigTextureRef(const TextureString& filename, TextureId id, Vec2 minPos = { 0.0f, 0.0f }, Vec2 maxPos = { 1.0f, 1.0f })
		:info{ filename }, TextureRef{id, minPos, maxPos}
	{}

	TextureDiscriptor info;
};

inline TextureRef makeAtlasRef(int atlasId, const Vec2 atlasGridSize, const Vec2 startAtlasCell, const Vec2 endAtlasCell = Vec2{ 0,0 })
{
	TextureRef texRef;
	texRef.id = atlasId;
	texRef.minPos = startAtlasCell / atlasGridSize;
	if (endAtlasCell == Vec2{ 0,0 }) {
		texRef.maxPos = (startAtlasCell + Vec2(1, 1)) / atlasGridSize;
	}
	else {
		texRef.maxPos = (endAtlasCell + Vec2(1, 1)) / atlasGridSize;
	}
	return texRef;
}

inline TextureRef makeAsciiRef(int atlasTextureId, char c)
{
	TextureRef texRef;
	c -= 0x20;
	Vec2 atlasCoord{ float(c % 8), float(15 - c / 8) };
	texRef = makeAtlasRef(atlasTextureId, Vec2(8, 16), atlasCoord);
	return texRef;
}

/**
 * A Sprite is a colored Rectangle or a Circle, that can optinaly be textured.
 */
struct Sprite {
	Vec4 color{ 1.0f, 1.0f, 1.0f, 1.0f };
	Vec3 position;
	/* in radiants 2pi = one rotation*/
	RotaVec2 rotationVec;
	Vec2 scale{ 1.0f, 1.0f };
	std::optional<TextureRef> texRef;
	Form form{ Form::Rectangle };
	RenderSpace drawMode{ RenderSpace::WindowSpace };

	bool operator<(Sprite const& rhs) const
	{
		return this->position.z < rhs.position.z;
	}
};

inline static std::ostream& operator<<(std::ostream& os, const Sprite& sprite)
{
	os << "color: " << sprite.color <<
		", position:" << sprite.position <<
		", rotation: " << sprite.rotationVec.toUnitX0() <<
		", scale: " << sprite.scale <<
		", form: " << sprite.form;
	return os;
}

inline Sprite makeSprite(uint32_t id, Vec2 position, float drawingPrio, Vec2 scale, Vec4 color, Form form, RotaVec2 rotation, RenderSpace drawMode, TextureRef texRef)
{
	assert(texRef.id != -1);
	return Sprite{
		.color = color,
		.position = {position.x, position.y, drawingPrio},
		.rotationVec = rotation,
		.scale = scale,
		.texRef = texRef,
		.form = form,
		.drawMode = drawMode
	};
}

inline Sprite makeSprite(uint32_t id, Vec2 position, float drawingPrio, Vec2 scale, Vec4 color, Form form, RotaVec2 rotation, RenderSpace drawMode = RenderSpace::WorldSpace)
{
	auto squishDrawingPrio = [](float f) {
		float s = f < 0.0f ? -1.0f : 1.0f;
		return 0.5f * s * 1.0f - (1.0f / (std::fabs(f) + 1.0f)) + 0.5f;
	};

	return Sprite{
		.color = color,
		.position = {position.x, position.y, squishDrawingPrio(drawingPrio)},
		.rotationVec = rotation,
		.scale = scale,
		.form = form,
		.drawMode = drawMode
	};
}

struct Light {
	Vec4 color;
	Vec2 position;
	float radius;
};