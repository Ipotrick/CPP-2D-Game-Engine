#pragma once

#include <string>
#include <cassert>

#include "GL/glew.h"
#include "GLFW/glfw3.h"

#include "../math/Vec2.hpp"

class OpenGLFrameBuffer {
public:
	void initialize(uint32_t width = 1, uint32_t height = 1);
	void reset();
	void clear();
	void bind();

	void resize(uint32_t width, uint32_t height);
	std::pair<uint32_t, uint32_t> getSize() const { return { width, height }; }
	void setMinOp(GLuint op);
	void setMagOp(GLuint op);

	GLuint getTex() { return tex; }
	GLuint getBuffer() { return fbo; }
protected:
	uint32_t width = 1;
	uint32_t height = 1;
	GLuint tex = -1;
	GLuint fbo = -1;
	GLuint depthBuffer = -1;
};