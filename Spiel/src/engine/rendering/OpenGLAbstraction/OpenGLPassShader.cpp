#include "OpenGLPassShader.hpp"

namespace gl {

	struct PassShaderVertex {
		Vec2 cornerPos;
		Vec2 samplerCoord;
	};

	inline constexpr GLuint PASS_SHADER_INDICES[6] = { 0, 1, 2, 1, 2, 3 };

	inline constexpr PassShaderVertex PASS_SHADER_VERTECIES[4] = {
		PassShaderVertex{ idToCorner(0, { -1,-1 }, { 1,1 }), idToCorner(0, { 0, 0 }, { 1,1 }) },
		PassShaderVertex{ idToCorner(1, { -1,-1 }, { 1,1 }), idToCorner(1, { 0, 0 }, { 1,1 }) },
		PassShaderVertex{ idToCorner(2, { -1,-1 }, { 1,1 }), idToCorner(2, { 0, 0 }, { 1,1 }) },
		PassShaderVertex{ idToCorner(3, { -1,-1 }, { 1,1 }), idToCorner(3, { 0, 0 }, { 1,1 }) },
	};

	const char* PASS_VERTEX_SHADER{
		R"( 
		#version 460 core

		layout(location = 0) in vec2 corner;
		layout(location = 1) in vec2 uv;

		out vec2 v_uv;

		void main()
		{
			v_uv = uv;
			gl_Position = vec4(corner.x, corner.y, 0.0f, 1.0f);
		}
		)"
	};

	const char* PASS_FRAGMENT_DEFAULT_SHADER{
		R"( 
		#version 460 core

		layout(location = 50) uniform sampler2D samplerSlot;

		in vec2 v_uv;

		void main() 
		{
			gl_FragColor = texture2D(samplerSlot, v_uv);
		}
		)"
	};
	void PassShader::init(const char* pFragmentShader)
	{
		const char* fragmentShader = pFragmentShader ? pFragmentShader : PASS_FRAGMENT_DEFAULT_SHADER;
		initialize(PASS_VERTEX_SHADER, fragmentShader);
		setVertexAttributes<Vec2, Vec2>();
		bufferIndices(6, (u32*)PASS_SHADER_INDICES);
		bufferVertices(4, (void*)PASS_SHADER_VERTECIES);
	}
	void PassShader::renderTexToFBO(u32 textureGLID, u32 fboGLID, u32 minX, u32 minY, u32 maxX, u32 maxY)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, fboGLID);
		glUseProgram(program);
		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);

		// bind texture id to sampler slot 0
		GLuint const textureSlot = 0;
		glActiveTexture(GL_TEXTURE0 + 0);
		glBindTexture(GL_TEXTURE_2D, textureGLID);
		glUniform1i(50, textureSlot);

		glViewport(minX, minY, maxX, maxY);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, gl::PASS_SHADER_INDICES);
	}
	void PassShader::renderTexToFBO(u32 textureGLID, gl::Framebuffer& fbo)
	{
		fbo.bind();
		glUseProgram(program);
		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);

		// bind texture id to sampler slot 0
		GLuint const textureSlot = 0;
		glActiveTexture(GL_TEXTURE0 + 0);
		glBindTexture(GL_TEXTURE_2D, textureGLID);
		glUniform1i(50, textureSlot);

		glViewport(0, 0, fbo.getSize().first, fbo.getSize().second);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, gl::PASS_SHADER_INDICES);
	}
	void PassShader::renderToFBO(gl::Framebuffer& fbo)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, fbo.getBuffer());
		glUseProgram(program);
		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);

		glViewport(0, 0, fbo.getSize().first, fbo.getSize().second);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, gl::PASS_SHADER_INDICES);
	}
}
