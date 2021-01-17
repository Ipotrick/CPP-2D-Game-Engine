#include "TextureRefManager.hpp"

TextureRefManager::TextureRefManager()
{
	cacheTextureInfo(TextureDiscriptor("white"));
	cacheTextureInfo(TextureDiscriptor("default"));
	textureLoadingQueue.clear();
}

BigTextureRef TextureRefManager::makeRef(const TextureDiscriptor& texInfo, const Vec2 minPos, const Vec2 maxPos)
{
	auto refOpt = getCachedTextureRef(texInfo);
	auto ref =  
		refOpt.has_value()
		? refOpt.value()				// textureref is cached return it
		: cacheTextureInfo(texInfo);	// textureref must be cached first
	ref.minPos = minPos;
	ref.maxPos = maxPos;
	return ref;
}

void TextureRefManager::validate(BigTextureRef& ref)
{
	auto refOpt = getCachedTextureRef(ref.getInfo());
	auto validRef =
		refOpt.has_value()
		? refOpt.value()
		: cacheTextureInfo(ref.getInfo());
	ref.id = validRef.id;
}

BigTextureRef TextureRefManager::cacheTextureInfo(const TextureDiscriptor& texInfo)
{
	auto ref = BigTextureRef(texInfo, nextIndex++);
	texRefs[texInfo.name].push_back(ref);
	textureLoadingQueue.push_back(ref);
	return ref;
}

BigTextureRef TextureRefManager::cacheTextureInfo(TextureDiscriptor&& texInfo)
{
	auto ref = BigTextureRef(texInfo.name, nextIndex++);
	texRefs[texInfo.name].push_back(ref);
	textureLoadingQueue.push_back(ref);
	return ref;
}

std::optional<BigTextureRef> TextureRefManager::getCachedTextureRef(const TextureDiscriptor& texInfo)
{
	auto& texName = texInfo.name;
	// serach for texture ref:
	if (texRefs.contains(texName)) {
		// linear search for the right textureRef
		for (int i{ 0 }; i < texRefs[texName].size(); ++i) {
			auto& texRef = texRefs[texName];
			if (texRef[i].getInfo() == texInfo) {
				return texRef[i];
			}
		}
	}
	return {};
}
