#pragma once

#include <thread>

#include "../../math/vector_math.hpp"
#include "../../util/utils.hpp"
#include "../../types/ShortNames.hpp"

#include "OpenGLShaderUtil.hpp"
#include "OpenGLFramebuffer.hpp"

namespace gl {
	class Shader {
	public:
		void initialize(std::string const& vertexShaderSource, std::string const& fragmentShaderSource);

		void reset();

		void use();

		template<typename ... AttributeType>
		void setVertexAttributes()
		{
			assert(!bVertexTypeSet);
			bVertexTypeSet = true;

			this->vertexStructSize = (sizeof(AttributeType) + ...);
			this->vertexBufferSize = 1;

			glCreateBuffers(1, &vbo);
			glNamedBufferData(vbo, this->vertexStructSize * this->vertexBufferSize, nullptr, GL_DYNAMIC_DRAW);
			glBindBuffer(GL_ARRAY_BUFFER, vbo);

			glCreateVertexArrays(1, &vao);
			glBindVertexArray(vao);

			(setVertexAttribute<AttributeType>(), ...);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
		}

		template<typename ... AttributeType>
		void setInstanceVertexAttributes()
		{
			assert(!bInstanceVertexTypeSet);
			bInstanceVertexTypeSet = true;

			this->instanceStructSize = (sizeof(AttributeType) + ...);
			this->instanceBufferSize = 1;

			glCreateBuffers(1, &instanceVBO);
			glNamedBufferData(instanceVBO, this->instanceStructSize * this->instanceBufferSize, nullptr, GL_DYNAMIC_DRAW);
			glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);

			glBindVertexArray(vao);

			(setInstancedVertexAttribute<AttributeType>(), ...);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
		}

		void resizeVertexBuffer(u32 size)
		{
			this->vertexBufferSize = size;
			glNamedBufferData(vbo, this->vertexStructSize * this->vertexBufferSize, nullptr, GL_DYNAMIC_DRAW);
		}

		void resizeInstanceVertexBuffer(u32 size);

		void setUniform(int layoutLocation, s32 const* value, u32 count = 1);
		void setUniform(int layoutLocation, u32 const* value, u32 count = 1);
		void setUniform(int layoutLocation, f32 const* value, u32 count = 1);
		void setUniform(int layoutLocation, Vec2 const* value, u32 count = 1);
		void setUniform(int layoutLocation, Vec3 const* value, u32 count = 1);
		void setUniform(int layoutLocation, Vec4 const* value, u32 count = 1);
		void setUniform(int layoutLocation, Mat3 const* value, u32 count = 1);
		void setUniform(int layoutLocation, Mat4 const* value, u32 count = 1);

		void bufferIndices(u32 count, u32* indices);

		void bufferVertices(u32 vertexCount, void* vertexData);

		void bufferInstancedVertices(u32 vertexCount, void* vertexData);

		void draw(u32 indexCount, gl::Framebuffer& frameBuffer);

		void drawInstanced(u32 indexCount, u32 instances, gl::Framebuffer& frameBuffer);

		u32 getNativeHandle() const;
	protected:
		template<typename T>
		void setGLVertexAttrib(u32 stride, u8* offset) { static_assert(false); }

		template<> void setGLVertexAttrib<int>(u32 stride, u8* offset)
		{
			glVertexAttribIPointer(nextVertexAttribLocation, 1, GL_INT, stride, offset);
		}
		template<> void setGLVertexAttrib<float>(u32 stride, u8* offset)
		{
			glVertexAttribPointer(nextVertexAttribLocation, 1, GL_FLOAT, false, stride, offset);
		}
		template<> void setGLVertexAttrib<Vec2>(u32 stride, u8* offset)
		{
			glVertexAttribPointer(nextVertexAttribLocation, 2, GL_FLOAT, false, stride, offset);
		}
		template<> void setGLVertexAttrib<Vec3>(u32 stride, u8* offset)
		{
			glVertexAttribPointer(nextVertexAttribLocation, 3, GL_FLOAT, false, stride, offset);
		}
		template<> void setGLVertexAttrib<Vec4>(u32 stride, u8* offset)
		{
			glVertexAttribPointer(nextVertexAttribLocation, 4, GL_FLOAT, false, stride, offset);
		}


		template<typename T>
		void setVertexAttribute()
		{
			glEnableVertexAttribArray(nextVertexAttribLocation);
			setGLVertexAttrib<T>(vertexStructSize, nextVectexAttribOffset);
			nextVectexAttribOffset += sizeof(T);
			nextVertexAttribLocation += 1;
		}

		template<typename T>
		void setInstancedVertexAttribute()
		{
			glEnableVertexAttribArray(nextVertexAttribLocation);
			setGLVertexAttrib<T>(instanceStructSize, nextInstancedVectexAttribOffset);
			glVertexAttribDivisor(nextVertexAttribLocation, 1);
			nextInstancedVectexAttribOffset += sizeof(T);
			nextVertexAttribLocation += 1;
		}

		bool initialized() const { return bCompiled && bVertexTypeSet; }

		u32 ibo = -1;
		u32 vao = -1;
		u32 nextVertexAttribLocation{ 0 };

		u32 vbo = -1;
		u32 vertexStructSize{ 0 };
		u32 vertexBufferSize{ 0 };
		u8* nextVectexAttribOffset{ nullptr };

		u32 instanceVBO = -1;
		u32 instanceStructSize{ 0 };
		u32 instanceBufferSize{ 0 };
		u8* nextInstancedVectexAttribOffset{ nullptr };

		bool bCompiled{ false };
		bool bVertexTypeSet{ false };
		bool bInstanceVertexTypeSet{ false };
		bool bInstanceIndicesSet{ false };

		u32 program = -1;
	};
};
