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
	void initialize();
	bool loadTexture(std::string_view name);
	bool isTextureLoaded(std::string_view name);
	Texture& getTexture(std::string_view name);
private:
	std::string_view texturesPath;
	robin_hood::unordered_map<std::string_view, Texture> textureMap;
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

inline void checkGLError()
{
	GLenum err;
	while ((err = glGetError()) != GL_NO_ERROR) {
		std::cout << err;
	}
}