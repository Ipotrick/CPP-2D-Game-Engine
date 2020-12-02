#include "TextureRefManager.hpp"

TextureRefManager::TextureRefManager()
{
	cacheTextureInfo(TextureInfo("white"));
	cacheTextureInfo(TextureInfo("default"));
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

void TextureRefManager::validate(TextureRef2& ref)
{
	auto refOpt = getCachedTextureRef(ref.getInfo());
	auto validRef =
		refOpt.has_value()
		? refOpt.value()
		: cacheTextureInfo(ref.getInfo());
	ref.id = validRef.id;
}

TextureRef2 TextureRefManager::cacheTextureInfo(const TextureInfo& texInfo)
{
	auto ref = TextureRef2(texInfo, nextIndex++);
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
		for (int i{ 0 }; i < texRefs[texName].size(); ++i) {
			auto& texRef = texRefs[texName];
			if (texRef[i].getInfo() == texInfo) {
				return texRef[i];
			}
		}
	}
	return {};
}
