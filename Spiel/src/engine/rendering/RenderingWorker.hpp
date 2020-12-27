#pragma once
#include <mutex>
#include <condition_variable>
#include <array>

#include "GL/glew.h"
#include "GLFW/glfw3.h"

#include "OGLShaderUtil.hpp"
#include "OGLPassShader.hpp"
#include "RenderTypes.hpp"
#include "RenderBuffer.hpp"
#include "TextureCache.hpp"
#include "OGLTextureRenderBuffer.hpp"

struct SpriteShaderVertex {
	static int constexpr FLOAT_SIZE{ 5 };
	SpriteShaderVertex() :
		data{ 0 }
	{}
	union {
		struct {
			Vec2 corner;		// there are 4 corners: tl,tr,br,bl / (-1,1) (1,1) (1,-1) (-1-1)
			Vec2 texCoord;
			GLint modelIndex;
		};
		float data[FLOAT_SIZE];
	};
};

struct SpriteShaderModel {
	static int constexpr FLOAT_SIZE{ 16 };
	SpriteShaderModel() :
		data{ 0 }
	{}
	union {
		struct {
			Vec4 color;
			Vec4 position;
			Vec2 rotation;
			Vec2 scale;
			GLint texId;
			GLint isCircle;
			GLint renderSpace;
			uint32_t p0;		// padding for 16 byte alignment
			//uint32_t p1;		// padding for 16 byte alignment
			//uint32_t p2;		// padding for 16 byte alignment
		};
		float data[FLOAT_SIZE];
	};
};

struct SharedRenderData
{
	// sync  
	enum State {
		running,
		waitForFrontEnd,
		waitForStartCommand,
		reset
	};
	State state{ State::waitForStartCommand };
	bool run{ true };
	std::mutex mut{};
	std::condition_variable cond{};

	// render
	std::shared_ptr<RenderBuffer> renderBuffer{std::make_shared<RenderBuffer>()};

	// perf
	std::chrono::microseconds new_renderTime{ 0 };
	std::chrono::microseconds new_renderSyncTime{ 0 };
	int drawCallCount{ 0 };
};

class RenderingWorker
{
public:
	RenderingWorker(Window* wndw, std::shared_ptr<SharedRenderData> dt) :
		window{ wndw },
		data{ dt }
	{}

	void operator()();

	/**
	 * The following functions are the api for RenderingScripts:
	 */

	OGLTexFrameBuffer& getMainTFBO()
	{
		return mainTFBO;
	}

	Camera getCamera() { return data->renderBuffer->camera; }

	float getSuperSamplingFactor() const { return this->supersamplingFactor; }

	bool didWindowSizeChange() const { return bWindowSizeChanged; }

	Window* window;
private:
	// general:
	void initiate();
	void reset();
	/**
	 * waits for the frontend Renderer class to call waitForWorker.
	 * 
	 * \return is whether or not thee renderer should continue to exist
	 */
	bool waitForFrontend();

	// sprite rendering functions:
	size_t drawBatch(std::vector<Sprite>& drawables, Mat4 const& viewProjectionMatrix, Mat4 const& pixelProjectionMatrix, size_t startIndex);
	void drawLayer(RenderLayer& layer, Mat4 const& viewProjectionMatrix, Mat4 const& pixelProjectionMatrix);
	void generateVertices(Sprite const& d, float texID, Mat4 const& viewProjMat, Mat4 const& pixelProjectionMatrix, SpriteShaderVertex* bufferPtr);
	void bindTexture(GLuint texID, int slot = 0);

	// script managemenet functions:
	void newScriptsOnInitialize();
	void deadScriptsOnDestroy();

	// initialisation code:
	void initializeSpriteShader();

	void clearMainFBO();
private:

	uint32_t lastWindowWidth{ 0 };
	uint32_t lastWindowHeight{ 0 };
	bool bWindowSizeChanged{ true };
	float supersamplingFactor{ 2 };

	std::shared_ptr<SharedRenderData> data;

	TextureCache texCache{ "ressources/" };

	// configurations:
	inline static const int TEXTURE_WHITE = 0;
	inline static const int TEXTURE_DEFAULT = 1;
	int maxTextureSlots{};
	static constexpr GLuint BLEND_SFACTOR = GL_SRC_ALPHA;
	static constexpr GLuint BLEND_DFACTOR = GL_ONE_MINUS_SRC_ALPHA;

	// [T]exture based [F]rame [B]uffer [O]bject s:
	OGLTexFrameBuffer mainTFBO;

	// sprite shader:
	static constexpr size_t MAX_RECT_COUNT{ 1024 };	// max Rectangle Count
	static constexpr size_t MAX_VERTEX_COUNT{ MAX_RECT_COUNT * 4 };
	static constexpr size_t MAX_INDEX_COUNT{ MAX_RECT_COUNT * 6 };
	GLuint spriteShaderVBO = -1;
	GLuint spriteShaderVAO = -1;	// for struct Vertex2
	float* spriteShaderVBORaw{ nullptr };
	uint32_t* indicesRaw{ nullptr };
	inline static const int MODEL_SSBO_BINDING = 2;
	GLuint modelSSBO = -1;
	SpriteShaderModel* modelSSBORaw;
	GLuint modelUniformShaderBlockIndex = -1;
	int nextModelIndex{ 0 };
	GLuint spriteShaderProgram;
	std::string const SPRITE_SHADER_VERTEX_PATH = "shader/SpriteShader.vert";
	std::string const SPRITE_SHADER_FRAGMENT_PATH = "shader/SpriteShader.frag";

	// pass shader:
	std::string const PASS_SHADER_FRAGMENT_PATH = "shader/PassShader.frag";
	std::string const HDR_TONEMAP_SHADER_FRAGMENT_PATH = "shader/HDRToneMap.frag";
	OGLPassShader passShader;
};