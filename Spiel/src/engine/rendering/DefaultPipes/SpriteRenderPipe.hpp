#pragma once

#include "../pipeline/RenderPipeline.hpp"
#include "../pipeline/RenderPipelineThread.hpp"
#include "../pipeline/RenderRessourceManager.hpp"

#include "../Sprite.hpp"
#include "../SpriteShader.hpp"

#include "../OpenGLAbstraction/OpenGLShaderStorage.hpp"
#include "../OpenGLAbstraction/OpenGLTexture.hpp"

class SpriteRenderPipe : public IRenderPipe {
public:
	SpriteRenderPipe(TextureManager& tex, OpenGLShaderStorage<SpriteShaderModel>& modelSSBO) : tex{ tex }, backend{ tex.getBackend(), modelSSBO }
	{ }

	class Backend : public IRenderPipeBackend {
		inline static bool ssboInit{ false };
	public:
		Backend(TextureManager::Backend* texBackend, OpenGLShaderStorage<SpriteShaderModel>& modelSSBO) : texBackend{ texBackend }, modelSSBO{ modelSSBO } {
			texLock.unlock();
		}
		virtual void init(RenderPipeContext& context) override;
		virtual void reset(RenderPipeContext& context) override;
		virtual void render(RenderPipeContext& context) override;

		size_t drawBatch(std::vector<Sprite>& drawables, Mat4 const& viewProjectionMatrix, Mat4 const& pixelProjectionMatrix, size_t startIndex, OpenGLFrameBuffer& framebuffer);
		void generateVertices(Sprite const& d, int samplerSlot, Mat4 const& viewProjMat, Mat4 const& pixelProjectionMatrix, std::vector<SpriteShaderVertex>& vertexBuffer);
		void initializeSpriteShader();

		bool bInUse{ false };	// DEBUG SYMBOL
		std::vector<Sprite> spritesBack;
		int maxTextureSlots{};
		inline static const size_t MAX_RECT_COUNT{ 4096 };
		inline static const size_t MAX_VERTEX_COUNT{ MAX_RECT_COUNT * 4 };
		inline static const size_t MAX_INDEX_COUNT{ MAX_RECT_COUNT * 6 };
		inline static const std::string SPRITE_SHADER_VERTEX_PATH = "shader/SpriteShader.vert";
		inline static const std::string SPRITE_SHADER_FRAGMENT_PATH = "shader/SpriteShader.frag";
		inline static const int SPRITE_SHADER_MODEL_SSBO_BINDING = 2;
		inline static const std::string PASS_SHADER_FRAGMENT_PATH = "shader/PassShader.frag";
		inline static const std::string HDR_TONEMAP_SHADER_FRAGMENT_PATH = "shader/HDRToneMap.frag";
		TextureManager::Backend* texBackend{ nullptr };
		std::unique_lock<std::mutex> texLock{ texBackend->makeLock() };

		std::vector<uint32_t> spriteShaderIndices;
		std::vector<SpriteShaderVertex> spriteShaderCPUVertexBuffer;
		std::vector<SpriteShaderModel> spriteShaderCPUModelBuffer;

		OpenGLShader spriteShader;
		OpenGLShaderStorage<SpriteShaderModel>& modelSSBO;

		// SETTINGS:
		bool bClearDepthBeforeDraw{ false };
	};

	virtual void flush() override
	{
		assert(!backend.bInUse);
		std::swap(spritesFront, backend.spritesBack);
		spritesFront.clear();
		backend.bClearDepthBeforeDraw = bClearDepthBeforeDraw;
	}
	virtual IRenderPipeBackend* getBackend()  override
	{
		return &backend;
	}

	TextureManager& tex;
	std::vector<Sprite> spritesFront;
	Backend backend;
	bool bClearDepthBeforeDraw{ false };
};
