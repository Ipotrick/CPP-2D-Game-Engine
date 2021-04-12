#pragma once

#include <variant>
#include <stack>

#include "../pipeline/RenderPipeline.hpp"
#include "../pipeline/RenderPipelineThread.hpp"
#include "../pipeline/RenderRessourceManager.hpp"

#include "../Sprite.hpp"

#include "../OpenGLAbstraction/OpenGLShaderStorage.hpp"
#include "../OpenGLAbstraction/OpenGLTexture.hpp"
#include "../OpenGLAbstraction/Blending.hpp"
#include "../OpenGLAbstraction/DepthTest.hpp"

struct SpriteVertex {
	Vec2 cornerPos;
	Vec2 uv;
};

struct SpriteInstance {
	Vec4 color;
	Vec4 position;
	Vec2 rotation;
	Vec2 scale;
	Vec2 texMin;
	Vec2 texMax;
	Vec2 clipMin;			// GUI specific
	Vec2 clipMax;			// GUI specific
	GLint texSampler;
	GLint isMSDF;			// GUI specific
	float cornerRounding;	// GUI specific
	GLint renderSpace;
};

class SpritePipe : public IRenderPipe {
public:
	// mtsdf smoothing
	// mtsdf corner smoothing
	// 
	// color tint

	struct EndBatchCmd { };

	struct PopDepthTest { };

	struct PopBlendingFunction { };

	struct PushMTSDFSmoothing { f32 value{ 1.0f }; };
	struct PopMTSDFSmoothing{ };

	using Command = std::variant<
		Sprite,
		gl::DepthTest, 
		PopDepthTest, 
		gl::BlendingFunction, 
		PopBlendingFunction, 
		PushMTSDFSmoothing, 
		PopMTSDFSmoothing, 
		EndBatchCmd
	>;

	SpritePipe(TextureManager::Backend* texBackend);
	virtual void init(RenderPipeContext& context) override;
	virtual void reset(RenderPipeContext& context) override;
	virtual void render(RenderPipeContext& context) override;

	size_t drawBatch(std::vector<Command>& drawables, Mat4 const& viewProjectionMatrix, Mat4 const& pixelProjectionMatrix, size_t startIndex, gl::Framebuffer& framebuffer);
	void genInstance(Sprite const& d, int samplerSlot, Mat4 const& viewProjMat, Mat4 const& pixelProjectionMatrix, std::vector<SpriteInstance>& vertexBuffer);
	void initializeSpriteShader();

	bool bInUse{ false };
	std::vector<Command> commands;
	int maxTextureSlots{};
	inline static const size_t MAX_BATCH_SIZE{ 4096 };
	inline static const std::string SHADER_SRC_VERTEX_PATH = "shader/Sprite.vert";
	inline static const std::string SHADER_SRC_FRAGMENT_PATH = "shader/Sprite.frag";
	TextureManager::Backend* texBackend{ nullptr };
	std::unique_lock<std::mutex> texLock{ texBackend->makeLock() };

	std::stack<gl::BlendingFunction> blendingStack;
	std::stack<gl::DepthTest> depthTestStack;
	std::stack<f32> mtsdfSmoothingStack;

	f32 sdfBarrier{ 0.0f };

	std::vector<SpriteInstance> instances;

	u32 trisDrawn{ 0 };

	gl::Shader spriteShader;

	void push(std::vector<Command>& commandsIn);
};
