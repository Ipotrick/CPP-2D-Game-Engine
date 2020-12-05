#include "TextureCache.hpp"
#include "stb_image.hpp"

#include "GL/glew.h"
void checkGLError()
{
	GLenum err;
	while ((err = glGetError()) != GL_NO_ERROR) {
		std::cout << err;
	}
}

void TextureCache::reset()
{
	for (const auto& el : textures) {
		glDeleteTextures(1, &el.openglTexID);
	}
	textures.clear();
	loaded.clear();
	initialize();
}

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

bool TextureCache::loadTexture(const TextureRef2& ref)
{
	bool success{ false };
	std::cout << "isTextureLoaded(" << ref.getId() << ");\n";
	if (!isTextureLoaded(ref)) {
		printf("texid %i\n", ref.getId());
		std::cout << "load texture with id: " << ref.getId() << " and name: \"" << ref.getFilename() << "\"" << std::endl;

		Texture tex;
		tex.info = ref.getInfo();
		GLint minFilter = tex.info.minFilter;
		GLint magFilter = tex.info.magFilter;

		std::cout << "minFilter: " << minFilter << std::endl;
		std::cout << "magFilter: " << magFilter << std::endl;


		GLint clampMethod = GL_CLAMP_TO_EDGE;

		std::string texName;
		texName.resize(ref.getFilename().size());
		for (int i = 0; i < ref.getFilename().size(); ++i) {
			texName[i] = ref.getFilename()[i];
		}
		if (texName.size() > 4) {
			if (texName[0] == '_' && texName[1] == 'p' && texName[2] == 'l' && texName[3] == '_') {
				magFilter = GL_LINEAR;
				texName = texName.substr(4, texName.size());
			}
		}
		if (texName.size() > 4) {
			if (texName[0] == '_' && texName[1] == 'p' && texName[2] == 'r' && texName[3] == '_') {
				clampMethod = GL_REPEAT;
				texName = texName.substr(4, texName.size());
			}
		}

		std::string path;
		path.append(texturesPath);
		path.append(texName);

		stbi_set_flip_vertically_on_load(1);
		stbi_uc* localBuffer 
			= stbi_load(path.c_str(), &tex.width, &tex.height, &tex.channelPerPixel, 4/*rgba*/);	// generate cpu texture data

		if (localBuffer != nullptr) {	// did loading the file work?
			glGenTextures(1, &tex.openglTexID);
			glBindTexture(GL_TEXTURE_2D, tex.openglTexID);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, clampMethod);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, clampMethod);
			if (minFilter == GL_LINEAR) {
				GLfloat fLargest;
				glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &fLargest);
				glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, fLargest);
			}

			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, tex.width, tex.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, localBuffer);
			glBindTexture(GL_TEXTURE_2D, 0);

			//stbi_image_free(localBuffer);	// free cpu texture data
			success = true;
			textures[ref.getId()] = tex;
		}
		else {
			std::cerr << "error: texture with path: \"" << path << "\" could not be loaded!" << std::endl;
			tex.good = false;
			success = false;
			textures[ref.getId()] = textures[1];	// error texture
		}
		loaded[ref.getId()] = true;
	}
	else {
		success = true;
	}
	return success;
}

bool TextureCache::isTextureLoaded(const SmallTextureRef& ref)
{
	return isTextureLoaded(ref.id);
}

bool TextureCache::isTextureLoaded(const TextureRef2& texRef)
{
	return isTextureLoaded(texRef.getId());
}

Texture& TextureCache::getTexture(TextureId texId)
{
	if (!isTextureLoaded(texId)) {
		std::cerr << "error: tried to use texture with invalid id!";
		exit(-1);
	}
	return textures[texId];
}

Texture& TextureCache::getTexture(const SmallTextureRef& ref)
{
	return getTexture(ref.id);
}

void TextureCache::cacheTextures(const std::vector<TextureRef2>& loadingQueue)
{
	for (auto const& ref : loadingQueue) {
		assert(!isTextureLoaded(ref));
		if (ref.getId() >= loaded.size()) {
			loaded.resize(ref.getId() + 1, false);
			textures.resize(ref.getId() + 1, Texture());
		}
		loadTexture(ref);
	}
}

bool TextureCache::isTextureLoaded(const TextureId id)
{
	return ((id >= 0) && (id < loaded.size())) && loaded[id];
}
