#pragma once

#include <iostream>

#include "GL/glew.h"
#include "stb_image.hpp"

#include "../../types/ShortNames.hpp"
#include "RenderRessourceManager.hpp"


struct TextureDescriptor2 {
	TextureDescriptor2() = default;
	TextureDescriptor2(const std::string& filepath, GLint minFilter = GL_NEAREST, GLint magFilter = GL_NEAREST)
		:filepath{ filepath }, minFilter{ minFilter }, magFilter{ magFilter }
	{ }
	bool operator==(const TextureDescriptor2& other) const
	{
		return this->filepath == other.filepath
			&& this->minFilter == other.minFilter
			&& this->magFilter == other.magFilter;
	}
	std::string filepath;
	GLint minFilter{ GL_NEAREST };
	GLint magFilter{ GL_NEAREST };
	GLint clampMethod{ GL_CLAMP_TO_EDGE };
};

namespace std {
	template<> struct hash<TextureDescriptor2> {
		std::size_t operator()(TextureDescriptor2 const& d) const noexcept
		{
			std::size_t h0 = std::hash<std::string>{}(d.filepath);
			std::size_t h1 = std::hash<s32>{}(d.minFilter);
			std::size_t h2 = std::hash<s32>{}(d.magFilter);
			std::size_t h3 = std::hash<s32>{}(d.clampMethod);
			return h0 ^ h1 ^ h2 ^ h3;
		}
	};
}

struct TextureHandle : public RessourceHandleBase { };

struct OpenGLTexture2 {
	GLuint openGLId{ 0xFFFFFFFF };
	int width{ -1 };
	int height{ -1 };
	int channelPerPixel{ -1 };
	bool bValid{ false };
	bool bLoaded{ false };
	TextureDescriptor2 desc;

	OpenGLTexture2() = default;
	OpenGLTexture2(const TextureDescriptor2& desc)
	{
		load(desc);
	}

	~OpenGLTexture2()
	{
		reset();
	}

	bool valid() const { return bValid; }

	void reset()
	{
		if (bValid && bLoaded) {
			bValid = false;
			bLoaded = false;

			GLuint textures[1] = { this->openGLId };
			glDeleteTextures(1, textures);
		}
	}

	void reload() { /*TODO*/ }

	void load(const TextureDescriptor2& d)
	{
		std::cout << "loading texture: " << d.filepath << std::endl;

		this->bValid = true;
		this->desc = d;

		stbi_set_flip_vertically_on_load(1);
		stbi_uc* localBuffer
			= stbi_load(this->desc.filepath.c_str(), &this->width, &this->height, &this->channelPerPixel, 4/*rgba*/);	// generate cpu texture data

		if (this->bLoaded = (localBuffer != nullptr)) {
			glGenTextures(1, &this->openGLId);
			glBindTexture(GL_TEXTURE_2D, this->openGLId);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, this->desc.minFilter);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, this->desc.magFilter);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, this->desc.clampMethod);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, this->desc.clampMethod);
			GLfloat fLargest;
			glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &fLargest);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, fLargest);

			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, this->width, this->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, localBuffer);
			glBindTexture(GL_TEXTURE_2D, 0);
			delete localBuffer;
		}
	}

	void bindToSampler(int samplerSlot)
	{
		assert(bValid && bLoaded);
		glActiveTexture(GL_TEXTURE0 + samplerSlot);
		glBindTexture(GL_TEXTURE_2D, openGLId);
	}
};
