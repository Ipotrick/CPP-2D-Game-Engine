#include "TextureRefManager.hpp"

TextureRefManager::TextureRefManager()
{
	cacheTextureInfo(TextureInfo(std::string_view("white")));
	cacheTextureInfo(TextureInfo(std::string_view("default")));
	textureLoadingQueue.clear();
}

TextureRef2 TextureRefManager::makeRef(const TextureInfo& texInfo, const Vec2 minPos, const Vec2 maxPos)
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

TextureRef2 TextureRefManager::cacheTextureInfo(const TextureInfo& texInfo)
{
	auto ref = TextureRef2(texInfo.name, nextIndex++);
	texRefs[texInfo.name].push_back(ref);
	textureLoadingQueue.push_back(ref);
	return ref;
}

TextureRef2 TextureRefManager::cacheTextureInfo(TextureInfo&& texInfo)
{
	auto ref = TextureRef2(texInfo.name, nextIndex++);
	texRefs[texInfo.name].push_back(ref);
	textureLoadingQueue.push_back(ref);
	return ref;
}

std::optional<TextureRef2> TextureRefManager::getCachedTextureRef(const TextureInfo& texInfo)
{
	auto& texName = texInfo.name;
	// serach for texture ref:
	if (texRefs.contains(texName)) {
		// linear search for the right textureRef
		int i{ 0 };
		for (; i < texRefs[texName].size(); ++i) {
			if (texRefs[texName][i].getInfo() == texInfo) {
				return texRefs[texName][i];
			}
		}
	}
	return {};
}
