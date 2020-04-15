#pragma once
#include <mutex>
#include <condition_variable>

#include "GL/glew.h"
#include "GLFW/glfw3.h"

#include "RenderTypes.h"
#include "TextureHandler.h"

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
		data{ dt },
		texHandler{ "ressources/" }
	{}

	void operator()();
	void initiate();
	void end();
public:
	std::shared_ptr<Window> window;
	std::shared_ptr<RenderingSharedData> data;
private:
	std::string readShader(std::string path_);
	void drawDrawable(Drawable const& d, mat4 const& viewProjectionMatrix);
	void bindTexture(GLuint texID, int slot = 0);
	void bindTexture(std::string_view name, int slot = 0);
private:
	TextureHandler texHandler;
	robin_hood::unordered_map<uint32_t, TextureRef> textureRefMap;

	int maxTextureSlots{};

	unsigned int shader{};
	unsigned int shadowShader{};
	std::string const vertexShaderPath = "shader/Basic.vert";
	std::string const fragmentShaderPath = "shader/Basic.frag";
	std::string const vertexShadowShaderPath = "shader/shadow.vert";
	std::string const fragmentShadowShaderPath = "shader/shadow.frag";
};

