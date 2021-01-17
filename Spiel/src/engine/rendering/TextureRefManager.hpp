#pragma once

#include "RenderTypes.hpp"


class TextureRefManager {
public:
	TextureRefManager();
	BigTextureRef makeRef(const TextureDiscriptor& texInfo, const Vec2 minPos = { 0.0f, 0.0f }, const Vec2 maxPos = { 1.0f, 1.0f });
	std::vector<BigTextureRef>& getTextureLoadingQueue() { return textureLoadingQueue; }
	void clearTextureLoadingQueue() { textureLoadingQueue.clear(); }
	void reset() { *this = TextureRefManager(); }
	void validate(BigTextureRef& ref);
private:
	BigTextureRef cacheTextureInfo(const TextureDiscriptor& texInfo);
	BigTextureRef cacheTextureInfo(TextureDiscriptor&& texInfo);
	std::optional<BigTextureRef> getCachedTextureRef(const TextureDiscriptor& texInfo);
	int nextIndex{ 0 };
	robin_hood::unordered_map<TextureString, std::vector<BigTextureRef>> texRefs;
	std::vector<BigTextureRef> textureLoadingQueue;
};