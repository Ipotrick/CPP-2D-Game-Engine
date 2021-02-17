#pragma once

#include "OpenGLAbstraction/OpenGLShader.hpp"

struct SpriteShaderVertex {
	static int constexpr FLOAT_SIZE{ 2 };
	union {
		struct {
			GLint cornerNr;		// four corners 0, 1, 2, 3. The idToCornder function gets the coords of the nr's
			GLint modelIndex;
		};
		float data[FLOAT_SIZE];
	};
};

#define SPRITE_SHADER_VERTEX_ATTRIBUTES \
	GLint, \
	GLint

struct SpriteShaderModel {
	static int constexpr FLOAT_SIZE{ 20 };
	SpriteShaderModel() :
		data{ 0 }
	{
	}
	union {
		struct {
			Vec4 color;
			Vec4 position;
			Vec2 rotation;
			Vec2 scale;
			Vec2 texMin;
			Vec2 texMax;
			GLint texSampler;
			GLint isMSDF;
			float cornerRounding;
			GLint renderSpace;
		};
		float data[FLOAT_SIZE];
	};
};