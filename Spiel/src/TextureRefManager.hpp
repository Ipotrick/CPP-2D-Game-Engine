#pragma once

#include "RenderTypes.hpp"


class TextureRefManager {
public:
	TextureRefManager();
	TextureRef2 makeRef(const TextureInfo& texInfo, const Vec2 minPos = { 0.0f, 0.0f }, const Vec2 maxPos = { 0.0f, 0.0f });
	std::vector<TextureRef2>& getTextureLoadingQueue() { return textureLoadingQueue; }
	void resetTextureLoadingQueue() { textureLoadingQueue.clear(); }
	void reset() { *this = TextureRefManager(); }
private:
	TextureRef2 cacheTextureInfo(const TextureInfo& texInfo);
	TextureRef2 cacheTextureInfo(TextureInfo&& texInfo);
	std::optional<TextureRef2> getCachedTextureRef(const TextureInfo& texInfo);
	int nextIndex{ 0 };
	robin_hood::unordered_map<TextureString, std::vector<TextureRef2>> texRefs;
	std::vector<TextureRef2> textureLoadingQueue;
};