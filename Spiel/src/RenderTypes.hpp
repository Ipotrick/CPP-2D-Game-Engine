#pragma once

#include <vector>
#include <string>

#include "GL/glew.h"

#include "BaseTypes.hpp"
#include "Vec2.hpp"
#include "Camera.hpp"
#include "Window.hpp"


class Drawable {
public:
	/* x y world coordinates, z depth*/
	Vec2 position;
	/* in radiants 2pi = one rotation*/
	RotaVec2 rotationVec;
	Vec4 color;
	Vec2 scale;
	float drawingPrio;
	uint32_t id;
	Form form;

	Drawable(uint32_t id_, Vec2 position_, float drawingPrio_, Vec2 scale_, Vec4 color_, Form form_, RotaVec2 rotation_, bool inWindowSpace = false) :
		position{ position_ },
		rotationVec{ rotation_ },
		drawingPrio{ drawingPrio_ },
		scale{ scale_ },
		color{ color_ },
		id{ id_ },
		form{ form_ },
		inWindowSpace{ inWindowSpace }
	{
	}

	inline bool isInWindowSpace() const { return inWindowSpace; }
private:
	bool inWindowSpace;
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
	TextureRef(std::string_view name = "default.png", Vec2 minPos_ = { 0,0 }, Vec2 maxPos_ = { 1,1 }) :
		textureName{ name },
		minPos{ minPos_ },
		maxPos{ maxPos_ }
	{}
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
	std::vector<std::pair<uint32_t, TextureRef>> newTextureRefs{};
	Camera camera{};
};
