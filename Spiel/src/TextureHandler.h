#pragma once

#include <string_view>

#include "stb_image.h"
#include "robin_hood.h"

#include "RenderTypes.h"

class TextureHandler {
public:
	TextureHandler(std::string_view texturePath_ = "") :
		texturesPath{ texturePath_ }
	{
	}
	~TextureHandler() {
		for (auto el : textureMap) {
			glDeleteTextures(1, &el.second.openglTexID);
		}
	}
	// returns id texture could be loaded
	// textureMap functions
	void initialize();
	bool loadTexture(std::string_view name);
	bool isTextureLoaded(std::string_view name);
	Texture& getTexture(std::string_view name);

	// textureRefMap functions
	void refreshRefMap(std::vector<std::pair<uint32_t, TextureRef>>& texRefs);
	bool hasTexture(uint32_t drawableID);
	TextureRef& getTexRef(uint32_t drawableID);
private:
	std::string_view texturesPath;
	robin_hood::unordered_map<std::string_view, Texture> textureMap;
	robin_hood::unordered_map<uint32_t, TextureRef> textureRefMap;
};

__forceinline bool TextureHandler::isTextureLoaded(std::string_view name)
{
	return textureMap.contains(name);
}

__forceinline Texture& TextureHandler::getTexture(std::string_view name) 
{
	if (!isTextureLoaded(name)) {
#ifdef _DEBUG
		std::cerr << "warning: called getTexture with \"" << name << "\". This texture was not loaded before use!\n", 
#endif
		loadTexture(name);
	}
	return textureMap[name];
}

__forceinline void checkGLError()
{
	GLenum err;
	while ((err = glGetError()) != GL_NO_ERROR) {
		std::cout << err;
	}
}

__forceinline bool TextureHandler::hasTexture(uint32_t drawableID)
{
	return textureRefMap.contains(drawableID);
}

__forceinline TextureRef& TextureHandler::getTexRef(uint32_t drawableID)
{
	assert(textureRefMap.contains(drawableID));
	return textureRefMap[drawableID];
}