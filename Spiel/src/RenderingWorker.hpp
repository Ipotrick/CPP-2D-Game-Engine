#pragma once
#include <mutex>
#include <condition_variable>
#include <array>

#include "GL/glew.h"
#include "GLFW/glfw3.h"

#include "RenderTypes.hpp"
#include "RenderBuffer.hpp"
#include "TextureCache.hpp"

struct Vertex {
	static int constexpr FLOAT_SIZE{ 5 };
	Vertex() :
		data{ 0 }
	{}
	union {
		struct {
			Vec2 corner;		// there are 4 corners: tl,tr,br,bl / (-1,1) (1,1) (1,-1) (-1-1)
			Vec2 texCoord;
			int modelIndex;		// there are 3 uniform arrays for the model: position[float 2], rotation[float 2], scale[float 2]
		};
		float data[FLOAT_SIZE];
	};
};

struct RenderModel {
	static int constexpr FLOAT_SIZE{ 16 };
	RenderModel() :
		data{ 0 }
	{
	}
	union {
		struct {
			Vec4 color;
			Vec2 position;
			Vec2 rotation;
			Vec2 scale;
			GLint texId;
			GLint isCircle;
			GLint renderSpace;
			uint32_t p0;
			uint32_t p1;
			uint32_t p2;
		};
		float data[FLOAT_SIZE];
	};
};

struct RenderingSharedData
{
	// sync  
	bool run{ true };
	bool ready{ false };
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
	RenderingWorker(Window* wndw, std::shared_ptr<RenderingSharedData> dt) :
		window{ wndw },
		data{ dt }
	{}

	~RenderingWorker() {
		free(spriteShaderVBORaw);
		free(indicesRaw);
	}

	void operator()();
	void initiate();
	void end();
public:
	Window* window;
	std::shared_ptr<RenderingSharedData> data;
	std::string readShader(std::string path_);
private:
	void generateVertices(Drawable const& d, float texID, Mat4 const& viewProjMat, Mat4 const& pixelProjectionMatrix, Vertex* bufferPtr);
	void drawLayer(RenderLayer& layer, Mat4 const& viewProjectionMatrix, Mat4 const& pixelProjectionMatrix);
	// returns the index after the last element that was drawn in the batch
	size_t drawBatch(std::vector<Drawable>& drawables, Mat4 const& viewProjectionMatrix, Mat4 const& pixelProjectionMatrix, size_t startIndex);
	void bindTexture(GLuint texID, int slot = 0);
	void newScriptsOnInitialize();
	void deadScriptsOnDestroy();
private:
	TextureCache texCache{ "ressources/" };
	static constexpr int TEXTURE_WHITE = 0;
	static constexpr int TEXTURE_DEFAULT = 1;

	int maxTextureSlots{};

	static constexpr size_t MAX_RECT_COUNT{ 2048 };	// max Rectangle Count
	static constexpr size_t MAX_VERTEX_COUNT{ MAX_RECT_COUNT * 4 };
	static constexpr size_t MAX_INDEX_COUNT{ MAX_RECT_COUNT * 6 };
	uint32_t spriteShaderVBO = -1;
	uint32_t spriteShaderVAO = -1;	// for struct Vertex2
	float* spriteShaderVBORaw{ nullptr };
	uint32_t* indicesRaw{ nullptr };

	inline static const int MODEL_SSBO_BINDING = 2;
	GLuint modelSSBO;
	RenderModel* modelSSBORaw;
	GLuint modelUniformShaderBlockIndex;


	int nextModelIndex{ 0 };

	unsigned int spriteShaderProgram;
	std::string const SPRITE_SHADER_VERTEX_PATH = "shader/SpriteShader.vert";
	std::string const SPRITE_SHADER_FRAGMENT_PATH = "shader/SpriteShader.frag";
};

