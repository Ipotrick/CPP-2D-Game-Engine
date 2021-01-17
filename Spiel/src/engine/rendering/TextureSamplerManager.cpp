#include "TextureSamplerManager.hpp"

TextureSamplerManager::TextureSamplerManager(TextureCache& texCache, size_t samplerCount)
	:texCache{ texCache }, SAMPLER_COUNT{ samplerCount }
{
	texToSampler.reserve(samplerCount * 2);	// make sure there are no reallocations
}

void TextureSamplerManager::clear()
{
	texToSampler.clear();
	nextSamplerSlot = 0;
}

std::optional<GLint> TextureSamplerManager::getSampler(TextureRef const& texRef)
{
	if (texToSampler.contains(texRef.id))	/* do have the texture allready bound to a sampler? */
	{
		return texToSampler[texRef.id];		/* then get the sampler slot and use it for this sprite too */
	}
	else if (nextSamplerSlot < SAMPLER_COUNT)	/* if we have sampler slots left, we try to get the texture id and bind it to a new sampler */
	{
		if (texCache.isTextureLoaded(texRef) && texCache.getTexture(texRef).good) {
			const int samplerSlot = nextSamplerSlot++;
			auto texture = texCache.getTexture(texRef);
			texture.bindToSampler(samplerSlot);
			texToSampler.insert({ texRef.id, samplerSlot });
			return samplerSlot;
		}
		else	/* the given texture ref is invalid, we set the sampler to the sampler holding the default/error texture */
		{
			return 0;
		}
	}
	else	/* we have no sampler slots left, we can not draw this sprite in the current batch. */
	{
		return {};
	}
}
