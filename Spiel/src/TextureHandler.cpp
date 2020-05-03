#include "TextureHandler.h"

#include "GL/glew.h"

void TextureHandler::initialize()
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
		textureMap.insert({ "white", tex });
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
		textureMap.insert({ "default", tex });
		free(defaultTex);
	}
}

bool TextureHandler::loadTexture(std::string_view name)
{
	bool success{ false };
	if (!isTextureLoaded(name)) {

		Texture tex;

		std::string path;
		path.append(texturesPath);
		path.append(name);

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
			textureMap.insert({ name, tex });
			success = true;
		}
		else {
#ifdef _DEBUG
			std::cerr << "error: texture with path: \"" << path << "\" could not be loaded!" << std::endl;
#endif
		}
	}
	else {
		success = true;
	}
	return success;
}

void TextureHandler::refreshRefMap(std::vector<std::pair<uint32_t, TextureRef>>& texRefs)
{
	textureRefMap.clear();
	for (auto& tex : texRefs) {
		if (!isTextureLoaded(tex.second.textureName)) {
			// load all not loaded  textures from file
			auto success = loadTexture(tex.second.textureName);
		}
		textureRefMap.insert({ tex.first, tex.second });
	}
}