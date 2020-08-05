#pragma once

#include <vector>
#include <string>
#include <optional>

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
	Texture() :
		openglTexID{ 0 },
		width{ -1 },
		height{ -1 },
		channelPerPixel{ -1 }
	{}
	Texture(GLuint id, int width_, int height_, int bitsPerPixel_) :
		openglTexID{ id },
		width{ width_ },
		height{ height_ },
		channelPerPixel{ bitsPerPixel_ } 
	{}

	GLuint openglTexID;
	int width;
	int height;
	int channelPerPixel;
};



struct TextureRef {
	std::string_view textureName;
	Vec2 minPos;
	Vec2 maxPos;
	TextureRef(std::string_view name = "invalid", Vec2 minPos_ = { 0,0 }, Vec2 maxPos_ = { 1,1 }) :
		textureName{ name },
		minPos{ minPos_ },
		maxPos{ maxPos_ }
	{}
};

struct AtlasRef {
	TextureRef texRef;
	AtlasRef(const std::string_view atlasName, const Vec2 atlasGridSize, const Vec2 startAtlasCell, const Vec2 endAtlasCell = Vec2{0,0}) {
		texRef.textureName = atlasName;
		texRef.minPos = startAtlasCell / atlasGridSize;
		if (endAtlasCell == Vec2{ 0,0 }) {
			texRef.maxPos = (startAtlasCell + Vec2(1, 1)) / atlasGridSize;
		}
		else {
			texRef.maxPos = (endAtlasCell + Vec2(1, 1)) / atlasGridSize;
		}
	}

	operator TextureRef() const {
		return texRef;
	}
};

struct AsciiRef {
	TextureRef texRef;
	AsciiRef(std::string_view asciiAtlasName, char c) {
		c -= 0x20;
		Vec2 atlasCoord(c % 8, 15 - c / 8);
		texRef = AtlasRef(asciiAtlasName, Vec2(8, 16), atlasCoord);
	}

	operator TextureRef() const {
		return texRef;
	}
};

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
		if (texRef.textureName == "invalid") {
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
