#pragma once

#include "OpenGLShader.hpp"

namespace gl {
	class PassShader : protected Shader {
	public:
		void init(const char* fragmentShader = nullptr);

		using Shader::setUniform;
		using Shader::reset;

		void renderTexToFBO(u32 textureGLID, u32 target, u32 minX, u32 minY, u32 maxX, u32 maxY);
		void renderTexToFBO(u32 textureGLID, gl::Framebuffer& fbo);
		void renderToFBO(gl::Framebuffer& fbo);
	};
}