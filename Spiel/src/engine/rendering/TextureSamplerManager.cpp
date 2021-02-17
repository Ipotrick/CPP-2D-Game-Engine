#include "TextureSamplerManager.hpp"

TextureSamplerManager::TextureSamplerManager(TextureManager::Backend& texBackend, std::unique_lock<std::mutex>& lock, size_t samplerCount)
	:texBackend{ texBackend }, SAMPLER_COUNT{ samplerCount }, lock{lock}
{
	texToSampler.reserve(samplerCount * 2);	// make sure there are no reallocations
}

void TextureSamplerManager::clear()
{
	texToSampler.clear();
	nextSamplerSlot = 0;
}

std::optional<u32> TextureSamplerManager::getSampler(const TextureHandle& handle)
{
	if (texToSampler.contains(handle.index))	/* do have the texture allready bound to a sampler? */
	{
		return texToSampler[handle.index];		/* then get the sampler slot and use it for this sprite too */
	}
	else if (nextSamplerSlot < SAMPLER_COUNT)	/* if we have sampler slots left, we try to get the texture id and bind it to a new sampler */
	{
		if (texBackend.isHandleValid(lock, handle) && texBackend.isLoaded(lock, handle.index)) {
			const int samplerSlot = nextSamplerSlot++;
			auto& texture = texBackend.get(lock, handle.index);
			texture.bindToSampler(samplerSlot);
			texToSampler.insert({ handle.index, samplerSlot });
			return samplerSlot;
		}
		else	/* the given texture ref is invalid, we set the sampler to the sampler holding the default/error texture */
		{
#ifdef _DEBUG
			std::cerr << "WARNING: " << "INVALID TEXTURE HANDLE WAS FOUND IN RENDERING!\n";
#endif
			return -2;
		}
	}
	else	/* we have no sampler slots left, we can not draw this sprite in the current batch. */
	{
		return {};
	}
}
