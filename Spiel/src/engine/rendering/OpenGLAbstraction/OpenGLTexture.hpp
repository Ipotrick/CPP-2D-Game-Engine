#pragma once

#include <iostream>

#include "../../types/ShortNames.hpp"
#include "../pipeline/RenderRessourceManager.hpp"

enum class TexFilter : u32 {
	Nearest,
	Linear,
};
std::ostream& operator<<(std::ostream& os, TexFilter filter);

enum class TexClamp : u32 {
	ToEdge
};
std::ostream& operator<<(std::ostream& os, TexClamp filter);

struct TextureDescriptor {
	TextureDescriptor() = default;
	TextureDescriptor(const std::string& filepath, TexFilter minFilter = TexFilter::Nearest, TexFilter magFilter = TexFilter::Nearest)
		:filepath{ filepath }, minFilter{ minFilter }, magFilter{ magFilter }
	{ }

	bool operator==(const TextureDescriptor& other) const
	{
		return this->filepath == other.filepath
			&& this->minFilter == other.minFilter
			&& this->magFilter == other.magFilter;
	}

	bool holdsValue() const { return filepath.size() > 0; }

	std::string filepath;
	TexFilter minFilter{ TexFilter::Nearest };
	TexFilter magFilter{ TexFilter::Nearest };
	TexClamp clampMethod{ TexClamp::ToEdge };
};
std::ostream& operator<<(std::ostream& os, const TextureDescriptor& d);

namespace std {
	template<> struct hash<TextureDescriptor> {
		std::size_t operator()(TextureDescriptor const& d) const noexcept
		{
			std::size_t h0 = std::hash<std::string>{}(d.filepath);
			std::size_t h1 = std::hash<TexFilter>{}(d.minFilter);
			std::size_t h2 = std::hash<TexFilter>{}(d.magFilter);
			std::size_t h3 = std::hash<TexClamp>{}(d.clampMethod);
			return h0 ^ h1 ^ h2 ^ h3;
		}
	};
}

struct TextureHandle : public RessourceHandleBase {
	bool operator==(const TextureHandle& rhs) const
	{
		return *reinterpret_cast<u64 const*>(this) == *reinterpret_cast<u64 const*>(&rhs);
	}
};

struct OpenGLTexture {
	OpenGLTexture() = default;
	OpenGLTexture(const OpenGLTexture& rhs) = delete;
	OpenGLTexture(OpenGLTexture&& rhs);
	OpenGLTexture(const TextureDescriptor& desc);

	~OpenGLTexture();

	void load(const TextureDescriptor& d);

	void reload();

	bool loaded() const { return openGLId != 0xFFFFFFFF; }

	void reset();

	void bindToSampler(int samplerSlot) const;

	u32 openGLId{ 0xFFFFFFFF };
	int width{ -1 };
	int height{ -1 };
	int channelPerPixel{ -1 };
	TextureDescriptor desc;
};

using TextureManager = RenderRessourceManager<TextureHandle, TextureDescriptor, OpenGLTexture>;