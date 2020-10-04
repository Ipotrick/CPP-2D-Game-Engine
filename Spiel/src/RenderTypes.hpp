#pragma once

#include <vector>
#include <string>
#include <optional>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/static_string.hpp>

#include "robin_hood.h"

#include "GL/glew.h"

#include "BaseTypes.hpp"
#include "Vec2.hpp"
#include "Camera.hpp"
#include "Window.hpp"

enum class DrawMode : char {
	/*world coordinates, (0,0) is world's (0,0)*/
	WorldSpace,
	/* window (-1 to 1 in x and y) cooordinates, (0,0) is middle */
	WindowSpace,
	/* window coordinates that ignore aspect ratio, (0,0) is middle*/
	UniformWindowSpace,
	/* coordinates are pixels, (0,0) is lower left corner */
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

struct TextureId {
	int index{ -1 };
	int version{ 0 };
	void reset()
	{
		index = -1;
		version = 0;
	}
};

class TextureRef2 {
public:

	TextureRef2(const std::string& filename = "default", Vec2 minPos = { 0,0 }, Vec2 maxPos = { 0,0 })
		:filename{ filename }, minPos{ minPos }, maxPos{ maxPos }
	{}
	TextureRef2(const std::string_view& filename, Vec2 minPos = { 0,0 }, Vec2 maxPos = { 0,0 })
		:minPos{ minPos }, maxPos{ maxPos }
	{
		this->filename.resize(std::min(filename.size(), size_t(32)));
		for (int i = 0; i < 32 && i < filename.size(); i++) {
			this->filename.at(i) = filename.at(i);
		}
	}

	void setFilename(const std::string& filename)
	{
		this->filename = filename;
		id.reset();
	}
	const boost::static_string<32>& getFilename() const
	{
		return filename;
	}
	void setId(TextureId texId)
	{
		this->id = texId;
	}
	TextureId getId() const
	{
		return this->id;
	}

	Vec2 minPos{ 0.0f, 0.0f };
	Vec2 maxPos{ 1.0f, 1.0f };
private:
	boost::static_string<32> filename{ "default" };
	TextureId id;
	friend class boost::serialization::access;

	template<class Archive>
	void serialize(Archive& ar, const unsigned int file_version)
	{
		ar& filename;
		ar& minPos;
		ar& maxPos;
	}
};


struct SmallTextureRef {
	SmallTextureRef(int textureId = -1, Vec2 minPos_ = Vec2{ 0.001f, 0.001f }, Vec2 maxPos_ = Vec2{ 0.999f, 0.999f }) :
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

inline TextureRef2 makeAtlasRef2(const std::string_view& name, const Vec2 atlasGridSize, const Vec2 startAtlasCell, const Vec2 endAtlasCell = Vec2{ 0,0 })
{
	TextureRef2 texRef(name);
	texRef.minPos = startAtlasCell / atlasGridSize;
	if (endAtlasCell == Vec2{ 0,0 }) {
		texRef.maxPos = (startAtlasCell + Vec2(1, 1)) / atlasGridSize;
	}
	else {
		texRef.maxPos = (endAtlasCell + Vec2(1, 1)) / atlasGridSize;
	}
	return texRef;
}

inline SmallTextureRef makeAtlasRef(int atlasId, const Vec2 atlasGridSize, const Vec2 startAtlasCell, const Vec2 endAtlasCell = Vec2{ 0,0 })
{
	SmallTextureRef texRef;
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

inline TextureRef2 makeAsciiRef2(const std::string_view& name, char c)
{
	TextureRef2 texRef;
	c -= 0x20;
	Vec2 atlasCoord(c % 8, 15 - c / 8);
	texRef = makeAtlasRef2(name, Vec2(8, 16), atlasCoord);
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

	Drawable(uint32_t id_, Vec2 position_, float drawingPrio_, Vec2 scale_, Vec4 color_, Form form_, RotaVec2 rotation_, DrawMode drawMode = DrawMode::WorldSpace, SmallTextureRef texRef = {}) :
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
