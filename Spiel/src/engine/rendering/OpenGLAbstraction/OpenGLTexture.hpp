#pragma once

#include <iostream>

#include "../../types/ShortNames.hpp"
#include "../../math/Vec2.hpp"
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

struct TextureSettings {
	TexFilter minFilter{ TexFilter::Nearest };
	TexFilter magFilter{ TexFilter::Nearest };
	TexClamp clampMethod{ TexClamp::ToEdge };
};

struct TextureLoadInfo {
	bool operator==(const TextureLoadInfo& other) const
	{
		return this->filepath == other.filepath &&
			this->minFilter == other.minFilter &&
			this->magFilter == other.magFilter &&
			this->clampMethod == other.clampMethod;
	}

	bool holdsValue() const { return filepath.size() > 0; }

	TextureSettings getSettings() const
	{
		return TextureSettings{ minFilter, magFilter, clampMethod };
	}

	std::string filepath;
	TexFilter minFilter{ TexFilter::Nearest };
	TexFilter magFilter{ TexFilter::Nearest };
	TexClamp clampMethod{ TexClamp::ToEdge };
};
std::ostream& operator<<(std::ostream& os, const TextureLoadInfo& d);
namespace std {
	template<> struct hash<TextureLoadInfo> {
		std::size_t operator()(TextureLoadInfo const& d) const noexcept
		{
			std::size_t h0 = std::hash<std::string>{}(d.filepath);
			std::size_t h1 = std::hash<TexFilter>{}(d.minFilter);
			std::size_t h2 = std::hash<TexFilter>{}(d.magFilter);
			std::size_t h3 = std::hash<TexClamp>{}(d.clampMethod);
			return h0 ^ h1 ^ h2 ^ h3;
		}
	};
}

struct TextureName {
	std::string name;

	operator std::string& ()
	{
		return name;
	}
	operator const std::string& () const
	{
		return name;
	}
};
namespace std {
	template<> struct hash<TextureName> {
		std::size_t operator()(TextureName const& d) const noexcept
		{
			return std::hash<std::string>{}(d.name);
		}
	};
}

struct TextureCreateInfo {
	bool operator==(const TextureCreateInfo& other) const
	{
		return this->data == other.data &&
			this->minFilter == other.minFilter &&
			this->magFilter == other.magFilter &&
			this->width == other.width &&
			this->height == other.height &&
			this->clampMethod == other.clampMethod;
	}

	bool holdsValue() const { return data != nullptr; }

	TextureSettings getSettings() const {
		return TextureSettings{ minFilter, magFilter, clampMethod };
	}

	std::unique_ptr<u8> data{ nullptr };
	u32 width{ 1 };
	u32 height{ 1 };
	TexFilter minFilter{ TexFilter::Nearest };
	TexFilter magFilter{ TexFilter::Nearest };
	TexClamp clampMethod{ TexClamp::ToEdge };
};
std::ostream& operator<<(std::ostream& os, const TextureCreateInfo& d);

struct TextureHandle : public RessourceHandleBase {
	bool operator==(const TextureHandle& rhs) const
	{
		return *reinterpret_cast<u64 const*>(this) == *reinterpret_cast<u64 const*>(&rhs);
	}
};

struct TextureSection {
	TextureHandle handle;
	Vec2 min{ 0,0 };
	Vec2 max{ 1,1 };
};

struct OpenGLTexture {
	OpenGLTexture() = default;
	OpenGLTexture(const OpenGLTexture& rhs) = delete;
	OpenGLTexture(OpenGLTexture&& rhs);
	OpenGLTexture(const TextureLoadInfo& desc); 
	OpenGLTexture(TextureCreateInfo& desc);

	~OpenGLTexture();

	void load(TextureSettings d, std::unique_ptr<u8>& data, u32 width, u32 height);
	void load(const TextureLoadInfo& d);
	void load(TextureCreateInfo& d);

	void reload();

	bool loaded() const { return openGLId != 0xFFFFFFFF; }

	void reset();

	void bindToSampler(int samplerSlot) const;

	u32 openGLId{ 0xFFFFFFFF };
	s32 width{ -1 };
	s32 height{ -1 };
	s32 channelPerPixel{ -1 };
	TextureSettings settings;
	std::optional<TextureLoadInfo> desc;
};

using TextureManager = RenderRessourceManager<TextureHandle, TextureLoadInfo, TextureCreateInfo, OpenGLTexture>;