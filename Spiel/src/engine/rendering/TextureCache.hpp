#pragma once

#include <string>

#include "robin_hood.h"

#include "RenderTypes.hpp"
#include "OpenGLTexture.hpp"

class TextureCache {
public:
	static constexpr int DEFAULT_TEXTURE_ID{ 0 };

	TextureCache(std::string texturePath_ = "") :
		texturesPath{ texturePath_ }
	{
	}
	~TextureCache() 
	{
		for (const auto& el : textures) {
			glDeleteTextures(1, &el.openGLId);
		}
	}
	void reset();
	void initialize();
	bool loadTexture(const BigTextureRef& texRef);
	bool isTextureLoaded(const TextureRef& texRef);
	bool isTextureLoaded(s32 id);
	bool isTextureLoaded(const BigTextureRef& texRef);
	OpenGLTexture& getTexture(const TextureId texId);
	OpenGLTexture& getTexture(const TextureRef& ref);
	void cacheTextures(const std::vector<BigTextureRef>& loadingQueue);
private:
	std::string texturesPath;
	std::vector<OpenGLTexture> textures;
	std::vector<bool> loaded;
};