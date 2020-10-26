#pragma once

#include <vector>
#include <string>
#include <optional>
#include <boost/static_string.hpp>

#include "robin_hood.h"

#include "GL/glew.h"

#include "BaseTypes.hpp"
#include "Vec2.hpp"
#include "Camera.hpp"
#include "Window.hpp"
#include "RenderSpace.hpp"

struct IntColor {
	uint8_t r{ 0 }, g{ 0 }, b{ 0 }, a{ 0 };
	IntColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
		:r{r},g{g},b{b},a{a} {}
	IntColor(Vec4 color)
		:r{(uint8_t)(color.r*255.0f)}, g{ (uint8_t)(color.g * 255.0f) },b{ (uint8_t)(color.b * 255.0f) },a{ (uint8_t)(color.a * 255.0f) } {}
	operator Vec4() const {
		return  { r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f };
	}
};

using TextureString = boost::static_string<32>;


using TextureId = int;

class TextureInfo {
public:
	TextureInfo() = default;
	TextureInfo(std::string_view&& name)
		:name{ name.data(), name.size() }
	{ }

	TextureInfo(const std::string_view& name)
		:name{ name.data(), name.size() }
	{
	}

	TextureInfo(const TextureString& name)
		:name{ name }
	{ }
	TextureString name;

	bool operator==(const TextureInfo& other) const
	{
		return name == other.name;
	}
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
*	The min and max pos can be modified freely.
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
	Vec2 atlasCoord(c % 8, 15 - c / 8);
	texRef = makeAtlasRef(atlasTextureId, Vec2(8, 16), atlasCoord);
	return texRef;
}

class Drawable {
public:
	Vec2 position;
	/* in radiants 2pi = one rotation*/
	RotaVec2 rotationVec;
	IntColor color;
	Vec2 scale;
	float drawingPrio;
	uint32_t id;
	Form form;
	std::optional<SmallTextureRef> texRef;

	Drawable(uint32_t id_, Vec2 position_, float drawingPrio_, Vec2 scale_, Vec4 color_, Form form_, RotaVec2 rotation_, RenderSpace drawMode = RenderSpace::WorldSpace) :
		position{ position_ },
		rotationVec{ rotation_ },
		drawingPrio{ drawingPrio_ },
		scale{ scale_ },
		color{ color_ },
		id{ id_ },
		drawMode{ drawMode },
		form{ form_ },
		texRef{ std::nullopt }
	{
	}

	Drawable(uint32_t id_, Vec2 position_, float drawingPrio_, Vec2 scale_, Vec4 color_, Form form_, RotaVec2 rotation_, RenderSpace drawMode, SmallTextureRef texRef) 
		:Drawable(id_, position_, drawingPrio_, scale_, color_, form_, rotation_, drawMode) 
	{
		/*
		* never create a Drawable with a texref THAT WAS NOT CREATED BY THE RENDERER
		*/
		assert(texRef.id != -1);
		this->texRef = texRef;
	}


	inline RenderSpace getDrawMode() const { return drawMode; }
private:
	RenderSpace drawMode;
};

struct Light {
	Light(Vec2 pos, float rad, uint32_t id_, Vec4 col) : position{ pos }, radius{ rad }, id{ id_ }, color{ col } {}
	Vec2 position;
	float radius;
	uint32_t id;
	Vec4 color;
};

struct RenderBuffer {
	std::vector<Drawable> drawables{};
	std::vector<TextureRef2> textureLoadingQueue;
	bool resetTextureCache{ false };
	Camera camera{};
};

class TextDrawable {
public:
	enum class Alignment {
		Left,
		Center,
		Right
	};
private:
	std::string str;
	const Drawable& drawable;
	Alignment alignment;
public:
	TextDrawable(std::string str, const Drawable& drawable, Alignment align)
		:str{ std::move(str) }, drawable{ drawable }, alignment{ align } {}

};
