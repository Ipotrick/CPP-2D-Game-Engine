#include "OpenGLShader.hpp"

namespace gl {
	void Shader::initialize(std::string const& vertexShaderSource, std::string const& fragmentShaderSource)
	{
		assert(!bCompiled);
		bCompiled = true;
		program = createOGLShaderProgram(vertexShaderSource, fragmentShaderSource);

		glCreateBuffers(1, &ibo);
	}

	void Shader::reset()
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

	void Shader::use()
	{
		glUseProgram(program);
		glBindVertexArray(vao);
	}

	void Shader::resizeInstanceVertexBuffer(uint32_t size)
	{
		assert(bInstanceVertexTypeSet);
		this->instanceBufferSize = size;
		glNamedBufferData(instanceVBO, this->instanceStructSize * this->instanceBufferSize, nullptr, GL_DYNAMIC_DRAW);
	}

	void Shader::setUniform(int layoutLocation, s32 const* value, uint32_t count)
	{
		assert(initialized());
		glUseProgram(program);
		glUniform1iv(layoutLocation, count, value);
	}

	void Shader::setUniform(int layoutLocation, u32 const* value, uint32_t count)
	{
		assert(initialized());
		glUseProgram(program);
		glUniform1uiv(layoutLocation, count, value);
	}

	void Shader::setUniform(int layoutLocation, f32 const* value, uint32_t count)
	{
		assert(initialized());
		glUseProgram(program);
		glUniform1fv(layoutLocation, count, value);
	}

	void Shader::setUniform(int layoutLocation, Vec2 const* value, uint32_t count)
	{
		assert(initialized());
		glUseProgram(program);
		glUniform2fv(layoutLocation, count, reinterpret_cast<float const*>(value));
	}

	void Shader::setUniform(int layoutLocation, Vec3 const* value, uint32_t count)
	{
		assert(initialized());
		glUseProgram(program);
		glUniform3fv(layoutLocation, count, reinterpret_cast<float const*>(value));
	}

	void Shader::setUniform(int layoutLocation, Vec4 const* value, uint32_t count)
	{
		assert(initialized());
		glUseProgram(program);
		glUniform4fv(layoutLocation, count, reinterpret_cast<float const*>(value));
	}

	void Shader::setUniform(int layoutLocation, Mat3 const* value, uint32_t count)
	{
		assert(initialized());
		glUseProgram(program);
		glUniformMatrix3fv(layoutLocation, count, false, reinterpret_cast<float const*>(value));
	}

	void Shader::setUniform(int layoutLocation, Mat4 const* value, uint32_t count)
	{
		assert(initialized());
		glUseProgram(program);
		glUniformMatrix4fv(layoutLocation, count, false, reinterpret_cast<float const*>(value));
	}

	void Shader::bufferIndices(u32 count, u32* indices)
	{
		glNamedBufferData(ibo, sizeof(u32) * count, indices, GL_STATIC_DRAW);
	}

	void Shader::bufferVertices(uint32_t vertexCount, void* vertexData)
	{
		assert(bVertexTypeSet);
		if (vertexCount > vertexBufferSize) {
			resizeVertexBuffer(vertexCount);
			vertexBufferSize = vertexCount;
		}

		glNamedBufferSubData(vbo, 0, vertexStructSize * vertexCount, vertexData);
	}

	void Shader::bufferInstancedVertices(uint32_t vertexCount, void* vertexData)
	{
		assert(bInstanceVertexTypeSet);
		if (vertexCount > instanceBufferSize) {
			resizeInstanceVertexBuffer(vertexCount);
		}

		glNamedBufferSubData(instanceVBO, 0, instanceStructSize * vertexCount, vertexData);
	}

	void Shader::draw(uint32_t indexCount, gl::Framebuffer& frameBuffer)
	{
		assert(initialized());

		frameBuffer.bind();
		glUseProgram(program);
		glBindVertexArray(vao);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);

		glViewport(0, 0, frameBuffer.getSize().first, frameBuffer.getSize().second);
		glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, nullptr);
	}

	void Shader::drawInstanced(uint32_t indexCount, u32 instances, gl::Framebuffer& frameBuffer)
	{
		assert(initialized());
		assert(bInstanceVertexTypeSet);

		frameBuffer.bind();
		glUseProgram(program);
		glBindVertexArray(vao);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);

		glViewport(0, 0, frameBuffer.getSize().first, frameBuffer.getSize().second);
		glDrawElementsInstanced(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, nullptr, instances);
	}

	u32 Shader::getNativeHandle() const
	{
		return program;
	}
}
