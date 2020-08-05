#pragma once

#include <string_view>

#include "stb_image.h"
#include "robin_hood.h"

#include "RenderTypes.hpp"

class TextureCache {
public:
	TextureCache(std::string_view texturePath_ = "") :
		texturesPath{ texturePath_ }
	{
	}
	~TextureCache() {
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
	void cacheTextures(std::vector<Drawable>& drawables);
private:
	std::string_view texturesPath;
	robin_hood::unordered_map<std::string_view, Texture> textureMap;
};

__forceinline bool TextureCache::isTextureLoaded(std::string_view name)
{
	return textureMap.contains(name);
}

__forceinline Texture& TextureCache::getTexture(std::string_view name) 
{
	if (!isTextureLoaded(name)) {
#ifdef _DEBUG
		std::cerr << "warning: called getTexture with \"" << name << "\". This texture was not loaded before use!\n", 
#endif
		loadTexture(name);
	}
	return textureMap[name];
}

inline void TextureCache::cacheTextures(std::vector<Drawable>& drawables)
{
	for (auto const& d : drawables) {
		if (d.texRef.has_value() && !isTextureLoaded(d.texRef.value().textureName)) {
			loadTexture(d.texRef.value().textureName);
		}
	}
}

__forceinline void checkGLError()
{
	GLenum err;
	while ((err = glGetError()) != GL_NO_ERROR) {
		std::cout << err;
	}
}