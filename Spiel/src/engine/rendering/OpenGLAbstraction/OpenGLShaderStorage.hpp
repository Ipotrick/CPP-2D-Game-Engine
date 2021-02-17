#pragma once

#include "OpenGLShader.hpp"

template<typename T>
class OpenGLShaderStorage {
public:

	void initialize(size_t maxSize, int binding)
	{
		if (ssbo != -1) reset();
		this->maxSize = maxSize;
		glCreateBuffers(1, &ssbo);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
		glBufferData(GL_SHADER_STORAGE_BUFFER, this->maxSize * sizeof(T), nullptr, GL_DYNAMIC_COPY);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding, ssbo);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	}

	void reset()
	{
		if (ssbo != -1) {
			GLuint buffers[1] = { ssbo };
			glDeleteBuffers(1, buffers);
		}
		*this = OpenGLShaderStorage<T>();
	}

	size_t size() const { return maxSize; }

	void buffer(int size, T* data)
	{
		assert(size <= maxSize);
		assert(data);
		glNamedBufferSubData(ssbo, 0, sizeof(T) * size, data);
	}
private:
	size_t maxSize{ 0 };
	GLuint ssbo = -1;
};