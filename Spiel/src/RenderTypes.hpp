#pragma once

#include <vector>
#include <string>
#include <optional>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

#include "robin_hood.h"

#include "GL/glew.h"

#include "BaseTypes.hpp"
#include "Vec2.hpp"
#include "Camera.hpp"
#include "Window.hpp"

enum class DrawMode : char {
	/*world coordinates*/
	WorldSpace,
	/* window (-1 to 1 in x and y) cooordinates */
	WindowSpace,
	/* window coordinates that ignore aspect ratio*/
	UniformWindowSpace,
	/* coordinates are pixels */
	PixelSpace
};

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

struct Texture {
	Texture() = default;
	Texture(GLuint id, int width_, int height_, int bitsPerPixel_) :
		openglTexID{ id },
		width{ width_ },
		height{ height_ },
		channelPerPixel{ bitsPerPixel_ } 
	{}

	GLuint openglTexID{ 0 };
	int width{ -1 };
	int height{ -1 };
	int channelPerPixel{ -1 };
	bool good{ true };
};



struct TextureRef {
	TextureRef(int textureId = -1, Vec2 minPos_ = { 0,0 }, Vec2 maxPos_ = { 1,1 }) :
		minPos{ minPos_ },
		maxPos{ maxPos_ },
		textureId{ textureId }
	{
	}

	Vec2 minPos;
	Vec2 maxPos;
	int textureId;
private:
	friend class boost::serialization::access;

	template<class Archive>
	void serialize(Archive& ar, const unsigned int file_version)
	{
		ar & textureId;
		ar & minPos;
		ar & maxPos;
	}
};

inline TextureRef makeAtlasRef(int atlasId, const Vec2 atlasGridSize, const Vec2 startAtlasCell, const Vec2 endAtlasCell = Vec2{ 0,0 })
{
	TextureRef texRef;
	texRef.textureId = atlasId;
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
	std::optional<TextureRef> texRef;

	Drawable(uint32_t id_, Vec2 position_, float drawingPrio_, Vec2 scale_, Vec4 color_, Form form_, RotaVec2 rotation_, DrawMode drawMode = DrawMode::WorldSpace, TextureRef texRef = {}) :
		position{ position_ },
		rotationVec{ rotation_ },
		drawingPrio{ drawingPrio_ },
		scale{ scale_ },
		color{ color_ },
		id{ id_ },
		drawMode{ drawMode },
		form{ form_ }
	{
		if (texRef.textureId < 0) {
			this->texRef = {};
		}
		else {
			this->texRef = texRef;
		}
	}

	inline DrawMode getDrawMode() const { return drawMode; }
private:
	DrawMode drawMode;
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
	std::vector<std::string*> textureNames;
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
