#pragma once

#include <optional>

#include "Sprite.hpp"
#include "RenderScript.hpp"

class RenderingWorker;

enum class DepthTest : GLuint {
	Ignore = GL_ALWAYS,
	Less = GL_LESS,
	LessOrEqual = GL_LEQUAL,
	Greater = GL_GREATER,
	GreaterOrEqual = GL_GEQUAL,
};

class RenderLayer {
public:
	void push(Sprite&& d) { sprites.push_back(d); }

	void push(Sprite const& d) { sprites.push_back(d); }

	void push(std::vector<Sprite> const& in)
	{
		sprites.insert(sprites.end(), in.begin(), in.end());
	}

	std::vector<Sprite>& getSprites()
	{
		return sprites;
	}

	void clearSprites() 
	{ 
		sprites.clear();
	}

	void setSprites(std::vector<Sprite> const& s)
	{
		sprites = s;
	}

	/**
	 * Set new or override old script of the layer.
	 * 
	 * \param script is called on the renderer after the layer is drawn.
	 */
	void attachRenderScript(std::unique_ptr<RenderScript>&& script)
	{
		this->script = std::move(script);	// give over new script
	}

	/**
	 * deletes current renderscript and calls the delete function on the renderscript.
	 * 
	 */
	void detachRenderScript()
	{
		this->bScriptDetach = true;
		if (this->script) {
			this->script = nullptr;
		}
	}

	/**
	 * this copies the settings of a layer.
	 * The settings include every field except the script, the sprites.
	 * 
	 * \param other layer to copy the settings from
	 */
	void copySettings(RenderLayer const& other)
	{
		this->bEnable = other.bEnable;
		this->bSortForDepth = other.bSortForDepth;
		this->bClearEveryFrame = other.bClearEveryFrame;
		this->depthTest = other.depthTest;
		this->renderMode = other.renderMode;
	}

	/**
	 * if a layer is disabled, it will not be rendered and the script will not be executed.
	 */
	bool bEnable{ true };
	/**
	 * if depthsorting is enables the sprites are sorted prior to rendering. This ensures that transparency blending will work correctly
	 */
	bool bSortForDepth{ false };
	/**
	 * In render depth testing modi.
	 */
	DepthTest depthTest{ DepthTest::LessOrEqual };
	/**
	 * Toggles if all sprites are cleard each frame.
	 */
	bool bClearEveryFrame{ true };	// if set to false, the buffer will not be cleared between frames.

	float zNear{ -1.0f };
	float zFar{ 1.0f };

	RenderSpace renderMode{ RenderSpace::Window };

private:
	friend class Renderer;
	friend class RenderingWorker;

	std::vector<Sprite> sprites;

	bool bScriptDetach{ false };	// signal for destroying current Script
	// temporary storage when in the frontBuffer
	// but permanent storage when in backbuffer
	std::unique_ptr<RenderScript> script{ nullptr };	
};