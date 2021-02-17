#pragma once

#include <thread>

#include "../../math/vector_math.hpp"
#include "../../util/utils.hpp"
#include "../../types/ShortNames.hpp"

#include "OpenGLShaderUtil.hpp"
#include "OpenGLTextureRenderBuffer.hpp"

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
	void setVertexAttributes(uint32_t maxVertecies = 1024)
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

	void resizeVertexBuffer(uint32_t size)
	{
		this->maxVertecies = size;
		glNamedBufferData(vbo, this->vertexStructSize * this->maxVertecies, nullptr, GL_DYNAMIC_DRAW);
	}

	void setUniform(int layoutLocation, s32 const* value, uint32_t count = 1);
	void setUniform(int layoutLocation, u32 const* value, uint32_t count = 1);
	void setUniform(int layoutLocation, f32 const* value, uint32_t count = 1);
	void setUniform(int layoutLocation, Vec2 const* value, uint32_t count = 1);
	void setUniform(int layoutLocation, Vec3 const* value, uint32_t count = 1);
	void setUniform(int layoutLocation, Vec4 const* value, uint32_t count = 1);
	void setUniform(int layoutLocation, Mat3 const* value, uint32_t count = 1);
	void setUniform(int layoutLocation, Mat4 const* value, uint32_t count = 1);

	void bufferVertices(uint32_t vertexCount, void* vertexData);

	void renderTo(uint32_t indexCount, uint32_t* indices, OpenGLFrameBuffer& frameBuffer);

	int getNativeHandle() const
	{
		return program;
	}
protected:
	template<typename T>
	void setGLVertexAttrib() { static_assert(false); }
	template<> void setGLVertexAttrib<int>()
	{
		glVertexAttribIPointer(nextVertexAttribLocation, 1, GL_INT, vertexStructSize, nextVectexAttribOffset);
	}
	template<> void setGLVertexAttrib<float>()
	{
		glVertexAttribPointer(nextVertexAttribLocation, 1, GL_FLOAT, false, vertexStructSize, nextVectexAttribOffset);
	}
	template<> void setGLVertexAttrib<Vec2>()
	{
		glVertexAttribPointer(nextVertexAttribLocation, 2, GL_FLOAT, false, vertexStructSize, nextVectexAttribOffset);
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

	GLuint vertexStructSize{ 0 };
	GLuint maxVertecies{ 0 };
	GLuint nextVertexAttribLocation{ 0 };
	uint8_t* nextVectexAttribOffset{ 0 };

	bool bCompiled{ false };
	bool bVertexTypeSet{ false };

	GLuint vao = -1;
	GLuint vbo = -1;
	GLuint program = -1;
};