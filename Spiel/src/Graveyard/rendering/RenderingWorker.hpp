#pragma once
#include <mutex>
#include <condition_variable>
#include <array>

#include "GL/glew.h"
#include "GLFW/glfw3.h"

#include "Sprite.hpp"
#include "RenderBuffer.hpp"
#include "TextureSamplerManager.hpp"
#include "Window.hpp"
#include "Camera.hpp"

#include "OpenGLAbstraction/OpenGLShaderUtil.hpp"
#include "OpenGLAbstraction/OpenGLPassShader.hpp"
#include "OpenGLAbstraction/OpenGLTextureRenderBuffer.hpp"
#include "OpenGLAbstraction/OpenGLShader.hpp"
#include "OpenGLAbstraction/OpenGLShaderStorage.hpp"

#include "SpriteShader.hpp"
#include "OpenGLAbstraction/OpenGLTexture.hpp"

struct SharedRenderData
{
	Window* window{ nullptr };

	// render
	std::unique_ptr<RenderBuffer> renderBuffer{std::make_unique<RenderBuffer>()};

	// perf
	std::chrono::microseconds new_renderTime{ 0 };
	std::chrono::microseconds new_renderSyncTime{ 0 };
	size_t drawCallCount{ 0 };
	size_t spriteCount{ 0 };
};

class RenderingWorker
{
public:
	RenderingWorker(SharedRenderData* dt, TextureManager::Backend* texManager) :
		data{ dt }, texManager{ texManager }, texLock{texManager->makeLock()}
	{
		texLock.unlock();
	}

	void initialize();

	bool initialized() const { return bInitialized; }

	void update();

	void reset();		// TODO: BUGGED DOESNT WORK (TEXTURE CACHE IS NOT RESET)

	OpenGLFrameBuffer& getMainFramebuffer() { return mainFramebuffer; }

	Camera getCamera() { return data->renderBuffer->camera; }

	float getSuperSamplingFactor() const { return this->supersamplingFactor; }

	bool didWindowSizeChange() const { return bWindowSizeChanged; }

	std::pair<uint32_t, uint32_t> getWindowSize() const { return { lastWindowWidth, lastWindowHeight }; }

private:

	// sprite rendering functions:
	size_t drawBatch(std::vector<Sprite>& drawables, Mat4 const& viewProjectionMatrix, Mat4 const& pixelProjectionMatrix, size_t startIndex, OpenGLFrameBuffer& framebuffer);
	void drawLayer(RenderLayer& layer, Mat4 const& viewProjectionMatrix, Mat4 const& pixelProjectionMatrix);
	void generateVertices(Sprite const& d, int samplerSlot, Mat4 const& viewProjMat, Mat4 const& pixelProjectionMatrix, std::vector<SpriteShaderVertex>& vertexBuffer);

	// script managemenet functions:
	void newScriptsOnInitialize();
	void deadScriptsOnDestroy();

	// initialisation code:
	void initializeSpriteShader();
private:
	/// CONSTANTS:
	inline static const GLuint BLEND_SFACTOR = GL_SRC_ALPHA;
	inline static const GLuint BLEND_DFACTOR = GL_ONE_MINUS_SRC_ALPHA;
	inline static const size_t MAX_RECT_COUNT{ 4096 };
	inline static const size_t MAX_VERTEX_COUNT{ MAX_RECT_COUNT * 4 };
	inline static const size_t MAX_INDEX_COUNT{ MAX_RECT_COUNT * 6 };
	inline static const std::string SPRITE_SHADER_VERTEX_PATH = "shader/SpriteShader.vert";
	inline static const std::string SPRITE_SHADER_FRAGMENT_PATH = "shader/SpriteShader.frag";
	inline static const int SPRITE_SHADER_MODEL_SSBO_BINDING = 2;
	inline static const std::string PASS_SHADER_FRAGMENT_PATH = "shader/PassShader.frag";
	inline static const std::string HDR_TONEMAP_SHADER_FRAGMENT_PATH = "shader/HDRToneMap.frag";

	/// META FIELDS:
	Window* windowOfLastFrame{ nullptr };
	bool bInitialized{ false };
	uint32_t lastWindowWidth{ 0 };
	uint32_t lastWindowHeight{ 0 };
	bool bWindowSizeChanged{ true };
	float supersamplingFactor{ 2 };	
	SharedRenderData* data{ nullptr };
	int maxTextureSlots{};

	TextureManager::Backend* texManager{ nullptr };
	std::unique_lock<std::mutex> texLock;

	/// CPU buffers:
	std::vector<uint32_t> spriteShaderIndices;
	std::vector<SpriteShaderVertex> spriteShaderCPUVertexBuffer;
	std::vector<SpriteShaderModel> spriteShaderCPUModelBuffer;

	/// GPU Interface:
	OpenGLShader spriteShader;
	OpenGLShaderStorage<SpriteShaderModel> spriteShaderModelSSBO;
	OpenGLFrameBuffer mainFramebuffer;
	OpenGLPassShader passShader;
};