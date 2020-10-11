#pragma once

#include <string>

#include "stb_image.h"
#include "robin_hood.h"

#include "RenderTypes.hpp"

class TextureCache {
public:
	TextureCache(std::string texturePath_ = "") :
		texturesPath{ texturePath_ }
	{
	}
	~TextureCache() 
	{
		for (const auto& el : textures) {
			glDeleteTextures(1, &el.openglTexID);
		}
	}
	void reset();
	void initialize();
	bool loadTexture(const TextureRef2& texRef);
	bool isTextureLoaded(const SmallTextureRef& texRef);
	bool isTextureLoaded(const TextureRef2& texRef);
	Texture& getTexture(const TextureId texId);
	Texture& getTexture(const SmallTextureRef& ref);
	void cacheTextures(const std::vector<TextureRef2>& loadingQueue);
private:
	bool isTextureLoaded(const TextureId);
	std::string texturesPath;
	std::vector<Texture> textures;
	std::vector<bool> loaded;
};