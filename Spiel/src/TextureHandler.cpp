#include "TextureHandler.hpp"

#include "GL/glew.h"

void TextureCache::initialize()
{
	{
		uint8_t* whitePoint = (uint8_t*)malloc(sizeof(uint8_t) * 4);
		whitePoint[0] = 255;
		whitePoint[1] = 255;
		whitePoint[2] = 255;
		whitePoint[3] = 255;

		Texture tex;
		tex.width = 1; tex.height = 1;
		glGenTextures(1, &tex.openglTexID);
		glBindTexture(GL_TEXTURE_2D, tex.openglTexID);
		
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, tex.width, tex.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, whitePoint);
		glBindTexture(GL_TEXTURE_2D, 0);
		textures.push_back(tex);
		loaded.push_back(true);
		free(whitePoint);
	}
	{
		uint8_t* defaultTex = (uint8_t*)malloc(sizeof(uint8_t) * 4 * 8 * 8);
		for (int i = 0; i < 8; i++) {
			for (int j = 0; j < 8; j++) {
				if ((i % 2) ^ (j % 2)) {
					defaultTex[i * 8 * 4 + j * 4 + 0] = 255;
					defaultTex[i * 8 * 4 + j * 4 + 1] = 0;
					defaultTex[i * 8 * 4 + j * 4 + 2] = 255;
					defaultTex[i * 8 * 4 + j * 4 + 3] = 255;
				}
				else {
					defaultTex[i * 8 * 4 + j * 4 + 0] = 0;
					defaultTex[i * 8 * 4 + j * 4 + 1] = 0;
					defaultTex[i * 8 * 4 + j * 4 + 2] = 0;
					defaultTex[i * 8 * 4 + j * 4 + 3] = 255;
				}
			}
		}

		Texture tex;
		tex.width = 8; tex.height = 8;
		glGenTextures(1, &tex.openglTexID);
		glBindTexture(GL_TEXTURE_2D, tex.openglTexID);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, tex.width, tex.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, defaultTex);
		glBindTexture(GL_TEXTURE_2D, 0);
		textures.push_back(tex);
		loaded.push_back(true);
		free(defaultTex);
	}
}

bool TextureCache::loadTexture(int texId)
{
	bool success{ false };
	if (!isTextureLoaded(texId)) {
		std::cout << "load texture with id: " << texId << " and name: \"" << *textureNames->at(texId) << "\"" << std::endl;

		Texture tex;

		std::string path;
		path.append(texturesPath);
		path.append(*textureNames->at(texId));

		stbi_set_flip_vertically_on_load(1);
		stbi_uc* localBuffer = stbi_load(path.c_str(), &tex.width, &tex.height, &tex.channelPerPixel, 4/*rgba*/);	// generate cpu texture data

		if (localBuffer != nullptr) {	// did loading the file work?
			glGenTextures(1, &tex.openglTexID);
			glBindTexture(GL_TEXTURE_2D, tex.openglTexID);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, tex.width, tex.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, localBuffer);
			glBindTexture(GL_TEXTURE_2D, 0);

			//stbi_image_free(localBuffer);	// free cpu texture data
			success = true;
		}
		else {
			std::cerr << "error: texture with path: \"" << path << "\" could not be loaded!" << std::endl;
			tex.good = false;
			success = false;
		}
		textures[texId] = tex;
		loaded[texId] = true;
	}
	else {
		success = true;
	}
	return success;
}



bool TextureCache::isTextureLoaded(int texId)
{
	return ((texId >= 0) && (texId < loaded.size())) && loaded[texId];
}

Texture& TextureCache::getTexture(int texId)
{
	if (!isTextureLoaded(texId)) {
		std::cerr << "warning: called getTexture with \"" << *textureNames->at(texId) << "\". This texture was not loaded before use!\n",
		loadTexture(texId);
	}
	return textures[texId];
}

void TextureCache::cacheTextures(std::vector<Drawable>& drawables)
{
	for (auto const& d : drawables) {
		if (d.texRef.has_value()) {
			auto& ref = d.texRef.value();
			if (!isTextureLoaded(ref.textureId)) {
				if (ref.textureId >= loaded.size()) {
					loaded.resize(ref.textureId + 1, false);
					textures.resize(ref.textureId + 1, Texture(0, 1, 1, 4));
				}
				loadTexture(ref.textureId);
			}
		}
	}
}

void checkGLError()
{
	GLenum err;
	while ((err = glGetError()) != GL_NO_ERROR) {
		std::cout << err;
	}
}