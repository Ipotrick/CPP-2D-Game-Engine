#pragma once

#include <thread>

#include "../math/vector_math.hpp"

#include "OpenGLShaderUtil.hpp"
#include "OpenGLTextureRenderBuffer.hpp"
#include "../util/utils.hpp"

class OpenGLShader {
public:
	void initialize(std::string const& vertexShaderSource, std::string const& fragmentShaderSource);

	void reset();

	void use()
	{
		glUseProgram(program);
		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
	}

	template<typename ... AttributeType>
	void setVertexAttributes(size_t maxVertecies = 1024)
	{
		assert(!bVertexTypeSet);
		bVertexTypeSet = true;

		this->vertexStructSize = (sizeof(AttributeType) + ...);
		this->maxVertecies = maxVertecies;

		glCreateBuffers(1, &vbo);
		glNamedBufferData(vbo, this->vertexStructSize * this->maxVertecies, nullptr, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		
		glCreateVertexArrays(1, &vao);
		glBindVertexArray(vao);
		
		(setVertexAttribute<AttributeType>(), ...);		// Fold expressions are amazing
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	void resizeVertexBuffer(size_t size)
	{
		this->maxVertecies = size;
		glNamedBufferData(vbo, this->vertexStructSize * this->maxVertecies, nullptr, GL_DYNAMIC_DRAW);
	}

	void setUniform(int layoutLocation, int const* value,	size_t count = 1);
	void setUniform(int layoutLocation, float const* value, size_t count = 1);
	void setUniform(int layoutLocation, Vec2 const* value,	size_t count = 1);
	void setUniform(int layoutLocation, Vec3 const* value,	size_t count = 1);
	void setUniform(int layoutLocation, Vec4 const* value,	size_t count = 1);
	void setUniform(int layoutLocation, Mat3 const* value,	size_t count = 1);
	void setUniform(int layoutLocation, Mat4 const* value,	size_t count = 1);

	void bufferVertices(int vertexCount, void* vertexData);

	void renderTo(size_t indexCount, uint32_t* indices, OpenGLFrameBuffer& frameBuffer);

	int getNativeHandle() const
	{
		return program;
	}
protected:
	template<typename T>
	void setGLVertexAttrib() { static_assert(false); }
	template<> void setGLVertexAttrib<int>()
	{
		glVertexAttribIPointer(nextVertexAttribLocation, 1, GL_INT, vertexStructSize, (const void*)nextVectexAttribOffset);
	}
	template<> void setGLVertexAttrib<float>()
	{
		glVertexAttribPointer(nextVertexAttribLocation, 1, GL_FLOAT, false, vertexStructSize, (const void*)nextVectexAttribOffset);
	}
	template<> void setGLVertexAttrib<Vec2>()
	{
		glVertexAttribPointer(nextVertexAttribLocation, 2, GL_FLOAT, false, vertexStructSize, (const void*)nextVectexAttribOffset);
	}


	template<typename T>
	void setVertexAttribute() 
	{
		glEnableVertexAttribArray(nextVertexAttribLocation);
		setGLVertexAttrib<T>();
		nextVectexAttribOffset += sizeof(T);
		nextVertexAttribLocation += 1;
	}

	bool initialized() const { return bCompiled && bVertexTypeSet; }

	size_t vertexStructSize{ 0 };
	size_t maxVertecies{ 0 };
	size_t nextVertexAttribLocation{ 0 };
	size_t nextVectexAttribOffset{ 0 };

	bool bCompiled{ false };
	bool bVertexTypeSet{ false };

	GLuint vao = -1;
	GLuint vbo = -1;
	GLuint program = -1;
};