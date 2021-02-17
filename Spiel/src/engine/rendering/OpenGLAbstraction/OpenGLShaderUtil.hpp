#pragma once

#include <string>
#include <cassert>

#include "../../types/ShortNames.hpp"
#include "../../math/Vec2.hpp"

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

u32 compileOGLShader(unsigned int type_, std::string const& source);

u32 createOGLShaderProgram(std::string const& vertexShader, std::string const& fragmentShader);