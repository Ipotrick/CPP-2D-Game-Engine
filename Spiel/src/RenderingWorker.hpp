#pragma once
#include <mutex>
#include <condition_variable>
#include <array>

#include "GL/glew.h"
#include "GLFW/glfw3.h"


#include "RenderTypes.hpp"
#include "TextureHandler.hpp"

struct Vertex {
	static int constexpr floatCount{ 10 };
	Vec2 position;
	Vec4 color;
	Vec2 texCoord;
	float texID;
	float circle;

	float operator[](int i) {
		assert(i >= 0 && i <= 9);
		switch (i) {
		case 0:
			return position.x;
			break;
		case 1:
			return position.y;
			break;
		case 2:
			return color[0];
			break;
		case 3:
			return color[1];
			break;
		case 4:
			return color[2];
			break;
		case 5:
			return color[3];
			break;
		case 6:
			return texCoord.x;
			break;
		case 7:
			return texCoord.y;
			break;
		case 8:
			return texID;
			break;
		case 9:
			return circle;
			break;
		}
	}
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
};

class RenderingWorker
{
public:
	RenderingWorker(std::shared_ptr<Window> wndw, std::shared_ptr<RenderingSharedData> dt) :
		window{ wndw },
		data{ dt }
	{}

	~RenderingWorker() {
		free(verteciesRawBuffer);
		free(indices);
	}

	void operator()();
	void initiate();
	void end();
public:
	std::shared_ptr<Window> window;
	std::shared_ptr<RenderingSharedData> data;
private:
	std::string readShader(std::string path_);
	std::array<Vertex, 4> generateVertices(Drawable const& d, float texID, Mat3 const& viewProjMat);
	void drawDrawable(Drawable const& d, Mat3 const& viewProjectionMatrix);
	// returns the index after the last element that was drawn in the batch
	size_t drawBatch(std::vector<Drawable>& drawables, Mat3 const& viewProjectionMatrix, size_t startIndex);
	void bindTexture(GLuint texID, int slot = 0);
	void bindTexture(std::string_view name, int slot = 0);
private:
	TextureHandler texHandler{ "ressources/" };

	int maxTextureSlots{};

	size_t const maxRectCount{ 1000 };	// max Rectangle Count
	size_t const maxVertexCount{ maxRectCount * 4 };
	size_t const maxIndicesCount{ maxRectCount * 6 };
	uint32_t verteciesBuffer{ 0 };
	float* verteciesRawBuffer{ nullptr };
	uint32_t* indices{ nullptr };

	unsigned int shader{};
	unsigned int shadowShader{};
	std::string const vertexShaderPath = "shader/Basic.vert";
	std::string const fragmentShaderPath = "shader/Basic.frag";
	std::string const vertexShadowShaderPath = "shader/shadow.vert";
	std::string const fragmentShadowShaderPath = "shader/shadow.frag";
};

