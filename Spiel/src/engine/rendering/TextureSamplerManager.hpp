#pragma once

#include "robin_hood.h"

#include "TextureCache.hpp"

class TextureSamplerManager {
public:
	TextureSamplerManager(TextureCache& texCache, size_t samplerCount);

	void clear();

	size_t usedSamplers() const
	{
		return nextSamplerSlot;
	}

	/**
	 * returns a potentially reused texture sampler slot for the given texture.
	 * 
	 * \param texRef reference to the texture that a sampler is requested to
	 * \return {} when all samplers are in use. int >= 0 when sampler is found. int == 0 when texRef is invalid or the default texture.
	 */
	std::optional<GLint> getSampler(s32 texid);
private:
	TextureCache& texCache;
	size_t SAMPLER_COUNT;
	GLint nextSamplerSlot{ 0 };
	robin_hood::unordered_map<TextureId, GLuint> texToSampler;
};
