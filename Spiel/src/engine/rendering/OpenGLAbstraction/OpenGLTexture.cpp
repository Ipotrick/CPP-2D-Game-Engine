#include "OpenGLTexture.hpp"

#include <GL/glew.h>

#include <stb_image.hpp>

std::ostream& operator<<(std::ostream& os, const TextureLoadInfo& d)
{
	return os << "{ " << d.filepath << ", minFilter: " << d.minFilter << ", magFilter: " << d.magFilter << ", clampMode: " << d.clampMethod << " }";
}

std::ostream& operator<<(std::ostream& os, const TextureCreateInfo& d)
{
	return os << "{ datapointer: " << d.data.get() << ", minFilter: " << d.minFilter << ", magFilter: " << d.magFilter << ", clampMode: " << d.clampMethod << " }";
}

std::ostream& operator<<(std::ostream& os, TexFilter filter)
{
	switch (filter) {
	case TexFilter::Nearest: os << "Nearest"; break;
	case TexFilter::Linear: os << "Linear"; break;
	default: assert(false);
	}
	return os;
}

std::ostream& operator<<(std::ostream& os, TexClamp filter)
{
	switch (filter) {
	case TexClamp::ToEdge: os << "ToEdge"; break;
	default: assert(false);
	}
	return os;
}

GLuint getGLEquivilant(TexFilter filter)
{
	switch (filter) {
	case TexFilter::Linear: return GL_LINEAR;
	case TexFilter::Nearest: return GL_NEAREST;
	default: assert(false);
	}
	return 0;
}

GLuint getGLEquivilant(TexClamp clamp)
{
	switch (clamp) {
	case TexClamp::ToEdge: return GL_CLAMP_TO_EDGE;
	default: assert(false);
	}
	return 0;
}

OpenGLTexture::OpenGLTexture(OpenGLTexture&& rhs)
{
	reset();
	this->openGLId = rhs.openGLId;
	rhs.openGLId = 0xFFFFFFFF;
	this->width = rhs.width;
	this->height = rhs.height;
	this->channelPerPixel = rhs.channelPerPixel;
	this->desc = rhs.desc;
}

OpenGLTexture::OpenGLTexture(const TextureLoadInfo& desc)
{
	load(desc);
}

OpenGLTexture::OpenGLTexture(TextureCreateInfo& desc)
{
	load(desc);
}

OpenGLTexture::~OpenGLTexture()
{
	reset();
}

void OpenGLTexture::reset()
{
	if (loaded()) {
		GLuint textures[1] = { this->openGLId };
		glDeleteTextures(1, textures);
		openGLId = 0xFFFFFFFF;
	}
}

void OpenGLTexture::load(const TextureLoadInfo& d)
{
	this->desc = d;

	stbi_set_flip_vertically_on_load(1);

	s32 width{ 1 };
	s32 height{ 1 };
	std::unique_ptr<u8> data{ stbi_load(d.filepath.c_str(), &width, &height, &this->channelPerPixel, 4) };

	load(d.getSettings(), data, width, height);
}

void OpenGLTexture::load(TextureCreateInfo& d)
{
	load(d.getSettings(), d.data, d.width, d.height);
}

void OpenGLTexture::load(TextureSettings s, std::unique_ptr<u8>& data, u32 width, u32 height)
{
	if (data) {
		glGenTextures(1, &this->openGLId);
		glBindTexture(GL_TEXTURE_2D, this->openGLId);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, getGLEquivilant(s.minFilter));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, getGLEquivilant(s.magFilter));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, getGLEquivilant(s.clampMethod));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, getGLEquivilant(s.clampMethod));
		GLfloat fLargest;
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &fLargest);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, fLargest);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data.get());
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	if (loaded()) {
		std::cout << "loading succeded\n";
		data.reset();
		this->width = width;
		this->height = height;
		this->settings = s;
	}
	else {
		std::cout << "loading failed\n";
	}
}

void OpenGLTexture::bindToSampler(int samplerSlot) const
{
	assert(loaded());
	glActiveTexture(GL_TEXTURE0 + samplerSlot);
	glBindTexture(GL_TEXTURE_2D, openGLId);
}


void OpenGLTexture::reload() {
	reset();
	if (desc.has_value()) {
		load(desc.value());
	}
}
