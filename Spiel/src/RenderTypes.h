#pragma once

#include <vector>
#include <string>

#include "GL/glew.h"

#include "BaseTypes.h"
#include "Camera.h"
#include "Window.h"


class Drawable {
public:
	/* x y world coordinates, z depth*/
	vec2 position;
	/* in radiants 2pi = one rotation*/
	float rotation;
	vec4 color;
	vec2 scale;
	float drawingPrio;
	uint32_t id;
	Form form;

	Drawable(uint32_t id_, vec2 position_, float drawingPrio_, vec2 scale_, vec4 color_, Form form_, float rotation_, bool inWindowSpace = false) :
		position{ position_ },
		rotation{ rotation_ },
		drawingPrio{ drawingPrio_ },
		scale{ scale_ },
		color{ color_ },
		id{ id_ },
		form{ form_ },
		inWindowSpace{ inWindowSpace }
	{
	}

	inline vec2 getPos() const { return position; }
	inline float getRota() const { return rotation; }
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
	vec2 minPos;
	vec2 maxPos;
	TextureRef(std::string_view name = "default.png", vec2 minPos_ = { 0,0 }, vec2 maxPos_ = { 1,1 }) :
		textureName{ name },
		minPos{ minPos_ },
		maxPos{ maxPos_ }
	{}
};

struct Light {
	Light(vec2 pos, float rad, uint32_t id_, vec4 col) : position{ pos }, radius{ rad }, id{ id_ }, color{ col } {}
	vec2 position;
	float radius;
	uint32_t id;
	vec4 color;
};

struct RenderBuffer {
	std::vector<Drawable> drawables{};
	std::vector<std::pair<uint32_t, TextureRef>> newTextureRefs{};
	Camera camera{};
};
