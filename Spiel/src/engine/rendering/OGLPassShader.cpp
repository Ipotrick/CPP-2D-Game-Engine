#include "OGLPassShader.hpp"

const std::string PASS_VERTEX_SHADER = 
"#version 460 core\n"
"layout(location = 0) in vec2 corner;\n"
"layout(location = 1) in vec2 uv;\n"
"out vec2 v_uv;\n"
"void main()\n"
"{\n"
"	v_uv = uv;\n"
"	gl_Position = vec4(corner.x, corner.y, 0.0f, 1.0f);\n"
"}\n";

void OGLPassShader::initialize(std::string const& customFragmentShader)
{
	assert(!bInitialized);	// NEVER INITIALIZE A SHADER TWICE
	bInitialized = true;

	auto vertexShader = PASS_VERTEX_SHADER;// readShader(PASS_VERTEX_SHADER);
	auto fragmentShader = readShader(customFragmentShader);
	program = createOGLShaderProgram(vertexShader, fragmentShader);

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vao);
	glBufferData(GL_ARRAY_BUFFER, sizeof(PassShaderVertex) * 4, PASS_SHADER_VERTECIES, GL_STATIC_DRAW);

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	// corner position
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(PassShaderVertex), 0);
	// texture uv position
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(PassShaderVertex), (const void*)offsetof(PassShaderVertex, samplerCoord));
}

void OGLPassShader::reset()
{
	if (bInitialized) {
		glDeleteProgram(program);
		glDeleteVertexArrays(1, &vao);
		glDeleteBuffers(1, &vbo);
	}
	bInitialized = false;
}

void OGLPassShader::renderTexToFBO(GLuint textureGLID, GLuint fboGLID, uint32_t fboWidth, uint32_t fboHeight)
{
	assert(bInitialized);

	glBindFramebuffer(GL_FRAMEBUFFER, fboGLID);
	glUseProgram(program);
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	// bind texture id to sampler slot 0
	GLuint const textureSlot = 0;
	glActiveTexture(GL_TEXTURE0 + 0);
	glBindTexture(GL_TEXTURE_2D, textureGLID);
	glUniform1i(50, textureSlot);

	glViewport(viewPortOffset.x, viewPortOffset.y, fboWidth, fboHeight);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, PASS_SHADER_INDICES);
}

void OGLPassShader::renderTexToFBO(GLuint textureGLID, OGLTexFrameBuffer& tbo)
{
	renderTexToFBO(textureGLID, tbo.getBuffer(), tbo.getSize().first, tbo.getSize().second);
}

void OGLPassShader::renderToFBO(OGLTexFrameBuffer& fbo)
{
	assert(bInitialized);

	glBindFramebuffer(GL_FRAMEBUFFER, fbo.getBuffer());
	glUseProgram(program);
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	glViewport(viewPortOffset.x, viewPortOffset.y, fbo.getSize().first, fbo.getSize().second);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, PASS_SHADER_INDICES);
}

void OGLPassShader::setViewPortOffset(Vec2 offset)
{
	this->viewPortOffset = offset;
}

void OGLPassShader::bind()
{
	glUseProgram(program);
}
