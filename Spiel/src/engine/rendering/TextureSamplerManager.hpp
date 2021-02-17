#pragma once

#include "robin_hood.h"

#include "OpenGLAbstraction/OpenGLTexture.hpp"

class TextureSamplerManager {
public:
	TextureSamplerManager(TextureManager::Backend& texBackend, std::unique_lock<std::mutex>& lock, size_t samplerCount);

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
	std::optional<u32> getSampler(const TextureHandle& handle);
private:
	TextureManager::Backend& texBackend;
	std::unique_lock<std::mutex>& lock;
	size_t SAMPLER_COUNT;
	u32 nextSamplerSlot{ 0 };
	robin_hood::unordered_map<u32, u32> texToSampler;
};
