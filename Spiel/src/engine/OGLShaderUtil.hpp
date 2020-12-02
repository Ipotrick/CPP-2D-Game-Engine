#pragma once

#include <string>
#include <cassert>

#include "GL/glew.h"
#include "GLFW/glfw3.h"

#include "Vec2.hpp"

inline constexpr Vec2 idToCorner(const int id, const Vec2 min, const Vec2 max)
{
	switch (id) {
	case 0:
		return { min.x, max.y };	// tl
	case 1:
		return { max.x, max.y };	// tr
	case 2:
		return { min.x, min.y };	// bl
	case 3:
		return { max.x, min.y };	// br
	default:
		assert(false);
		return { 0,0 };
	}
}

std::string readShader(std::string const& path);

GLuint compileOGLShader(unsigned int type_, std::string const& source);

GLint createOGLShaderProgram(std::string const& vertexShader, std::string const& fragmentShader);