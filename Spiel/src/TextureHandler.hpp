#pragma once

#include <string>

#include "stb_image.h"
#include "robin_hood.h"

#include "RenderTypes.hpp"

class TextureCache {
public:
	TextureCache(std::vector<std::string*>* textureNames, std::string texturePath_ = "") :
		textureNames{ textureNames },
		texturesPath{ texturePath_ }
	{
	}
	~TextureCache() 
	{
		for (const auto& el : textures) {
			glDeleteTextures(1, &el.openglTexID);
		}
	}
	void initialize();
	bool loadTexture(int texId);
	bool isTextureLoaded(int texId);
	Texture& getTexture(int texId);
	void cacheTextures(std::vector<Drawable>& drawables);
	std::vector<std::string*>* textureNames;
private:
	std::string texturesPath;
	std::vector<Texture> textures;
	std::vector<bool> loaded;
};