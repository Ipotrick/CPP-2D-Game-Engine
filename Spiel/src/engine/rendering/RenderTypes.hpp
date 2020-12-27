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

class TextureInfo {
public:
	TextureInfo() = default;

	TextureInfo(const TextureString& name, GLint minFilter = GL_NEAREST, GLint magFilter = GL_NEAREST)
		:name{ name }, minFilter{ minFilter }, magFilter{ magFilter }
	{ }

	bool operator==(const TextureInfo& other) const
	{
		return this->name == other.name 
			&& this->minFilter == other.minFilter
			&& this->magFilter == other.magFilter;
	}

	TextureString name;
	GLint minFilter{ GL_NEAREST };
	GLint magFilter{ GL_NEAREST };
private:
};

struct Texture {
	Texture() = default;
	Texture(GLuint id, int width_, int height_, int bitsPerPixel_, TextureInfo info)
		:openglTexID{ id },
		width{ width_ },
		height{ height_ },
		channelPerPixel{ bitsPerPixel_ },
		info{ info }
	{}

	GLuint openglTexID{ 0 };
	int width{ -1 };
	int height{ -1 };
	int channelPerPixel{ -1 };
	bool good{ true };
	TextureInfo info;
};

struct SmallTextureRef {
	SmallTextureRef(int textureId = -1, Vec2 minPos_ = Vec2{ 0.001f, 0.001f }, Vec2 maxPos_ = Vec2{ 0.999f, 0.999f }) :
		minPos{ minPos_ },
		maxPos{ maxPos_ },
		id{ textureId }
	{
	}

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
class TextureRef2 : private SmallTextureRef {
public:
	const TextureString& getFilename() const
	{
		return info.name;
	}
	const TextureInfo& getInfo() const
	{
		return info;
	}
	TextureId getId() const
	{
		return this->id;
	}
	using SmallTextureRef::minPos;
	using SmallTextureRef::maxPos;

	TextureRef2()
		:info{"default"}, SmallTextureRef{}
	{}

	TextureRef2(const TextureString& filename, Vec2 minPos = { 0.0f, 0.0f }, Vec2 maxPos = { 1.0f, 1.0f })
		:info{ filename }, SmallTextureRef{ -1, minPos, maxPos }
	{}

	TextureRef2(TextureInfo info, SmallTextureRef ref)
		:info{ info }, SmallTextureRef{ ref }
	{}

	bool good() const { return id != -1;  }

	SmallTextureRef makeSmall() const
	{
		return SmallTextureRef{ id, minPos, maxPos };
	}
protected:
	friend class TextureRefManager;
	TextureRef2(const TextureString& filename, TextureId id, Vec2 minPos = { 0.0f, 0.0f }, Vec2 maxPos = { 1.0f, 1.0f })
		:info{ filename }, SmallTextureRef{id, minPos, maxPos}
	{}

	TextureInfo info;
};

inline SmallTextureRef makeAtlasRef(int atlasId, const Vec2 atlasGridSize, const Vec2 startAtlasCell, const Vec2 endAtlasCell = Vec2{ 0,0 })
{
	SmallTextureRef texRef;
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

inline SmallTextureRef makeAsciiRef(int atlasTextureId, char c)
{
	SmallTextureRef texRef;
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
	std::optional<SmallTextureRef> texRef;
	Form form{ Form::Rectangle };
	RenderSpace drawMode{ RenderSpace::WindowSpace };

	bool operator<(Sprite const& rhs) const
	{
		return this->position.z < rhs.position.z;
	}
};

inline Sprite makeSprite(uint32_t id, Vec2 position, float drawingPrio, Vec2 scale, Vec4 color, Form form, RotaVec2 rotation, RenderSpace drawMode, SmallTextureRef texRef)
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