#include "OpenGLTexture.hpp"

#include <GL/glew.h>

#include <stb_image.hpp>

std::ostream& operator<<(std::ostream& os, const TextureDescriptor& d)
{
	return os << "{ " << d.filepath << ", minFilter: " << d.minFilter << ", magFilter: " << d.magFilter << ", clampMode: " << d.clampMethod << " }";
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

OpenGLTexture::OpenGLTexture(const TextureDescriptor& desc)
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

void OpenGLTexture::load(const TextureDescriptor& d)
{
	this->desc = d;
	stbi_set_flip_vertically_on_load(1);

	if (stbi_uc* localBuffer
		= stbi_load(this->desc.filepath.c_str(), &this->width, &this->height, &this->channelPerPixel, 4)) {
		glGenTextures(1, &this->openGLId);
		glBindTexture(GL_TEXTURE_2D, this->openGLId);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, getGLEquivilant(this->desc.minFilter));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, getGLEquivilant(this->desc.magFilter));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, getGLEquivilant(this->desc.clampMethod));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, getGLEquivilant(this->desc.clampMethod));
		GLfloat fLargest;
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &fLargest);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, fLargest);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, this->width, this->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, localBuffer);
		glBindTexture(GL_TEXTURE_2D, 0);
		delete localBuffer;
	}

	std::cout << "loading " << (this->loaded() ? "succeded" : "failed") << std::endl;
}

void OpenGLTexture::bindToSampler(int samplerSlot) const
{
	assert(loaded());
	glActiveTexture(GL_TEXTURE0 + samplerSlot);
	glBindTexture(GL_TEXTURE_2D, openGLId);
}


void OpenGLTexture::reload() {
	reset();
	load(desc);
}
