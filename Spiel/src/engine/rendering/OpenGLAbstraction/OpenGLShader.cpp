#include "OpenGLShader.hpp"

void OpenGLShader::initialize(std::string const& vertexShaderSource, std::string const& fragmentShaderSource)
{
	assert(!bCompiled);
	bCompiled = true;
	program = createOGLShaderProgram(vertexShaderSource, fragmentShaderSource);
}

void OpenGLShader::reset()
{
	if (vao != -1) {
		GLuint buffers[1] = { vao };
		glDeleteBuffers(1, buffers);
		vao = -1;
	}
	if (vbo != -1) {
		GLuint buffers[1] = { vbo };
		glDeleteBuffers(1, buffers);
		vbo = -1;
	}
	if (program != -1) {
		glDeleteProgram(program);
		program = -1;
	}
	bCompiled = false;
	bVertexTypeSet = false;
}

void OpenGLShader::setUniform(int layoutLocation, s32 const* value, uint32_t count)
{
	assert(initialized());
	glUseProgram(program);
	glUniform1iv(layoutLocation, count, value);
}

void OpenGLShader::setUniform(int layoutLocation, u32 const* value, uint32_t count)
{
	assert(initialized());
	glUseProgram(program);
	glUniform1uiv(layoutLocation, count, value);
}

void OpenGLShader::setUniform(int layoutLocation, f32 const* value, uint32_t count)
{
	assert(initialized());
	glUseProgram(program);
	glUniform1fv(layoutLocation, count, value);
}

void OpenGLShader::setUniform(int layoutLocation, Vec2 const* value, uint32_t count)
{
	assert(initialized());
	glUseProgram(program);
	glUniform2fv(layoutLocation, count, reinterpret_cast<float const*>(value));
}

void OpenGLShader::setUniform(int layoutLocation, Vec3 const* value, uint32_t count)
{
	assert(initialized());
	glUseProgram(program);
	glUniform3fv(layoutLocation, count, reinterpret_cast<float const*>(value));
}

void OpenGLShader::setUniform(int layoutLocation, Vec4 const* value, uint32_t count)
{
	assert(initialized());
	glUseProgram(program);
	glUniform4fv(layoutLocation, count, reinterpret_cast<float const*>(value));
}

void OpenGLShader::setUniform(int layoutLocation, Mat3 const* value, uint32_t count)
{
	assert(initialized());
	glUseProgram(program);
	glUniformMatrix3fv(layoutLocation, count, false, reinterpret_cast<float const*>(value));
}

void OpenGLShader::setUniform(int layoutLocation, Mat4 const* value, uint32_t count)
{
	assert(initialized());
	glUseProgram(program);
	glUniformMatrix4fv(layoutLocation, count, false, reinterpret_cast<float const*>(value));
}

void OpenGLShader::bufferVertices(uint32_t vertexCount, void* vertexData)
{
	assert(bVertexTypeSet);
	if (vertexCount > maxVertecies) {
		resizeVertexBuffer(vertexCount);
	}

	glNamedBufferSubData(vbo, 0, vertexStructSize * vertexCount, vertexData);
}

void OpenGLShader::renderTo(uint32_t indexCount, uint32_t* indices, OpenGLFrameBuffer& frameBuffer)
{
	assert(initialized());

	frameBuffer.bind();
	glUseProgram(program);
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	glViewport(0, 0, frameBuffer.getSize().first, frameBuffer.getSize().second);
	glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, indices);
}