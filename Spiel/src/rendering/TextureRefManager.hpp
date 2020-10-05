#pragma once

#include "../RenderTypes.hpp"

class TextureRefManager {
public:
	void update(std::vector<Drawable>& drawables)
	{
		for (auto& d : drawables) {
			// does the drawable have a texture that needs to be updated?
			if (d.texRef.has_value()) {
				if ((d.texRef.value().getId().version != version || d.texRef.value().getId().index == -1)) {
					auto loadTex = [&]() 
					{
						d.texRef.value().setId(nextTexIndex, this->version);
						textureRefMap[d.texRef.value().getFilename().c_str()].push_back(d.texRef.value());
						loadingQueue.push_back(d.texRef.value());
						++nextTexIndex; 
					};

					// does the requested texture allready exist?
					if (textureRefMap.contains(d.texRef.value().getFilename().c_str())) {
						auto& refs = textureRefMap[d.texRef.value().getFilename().c_str()];
						int i{ 0 };
						for (auto& ref : refs) {
							if (ref.settings == d.texRef.value().settings) {
								break;
							}
							++i;
						}

						// texture is allready instanciated
						if (i < refs.size()) {
							d.texRef.value().setId(i, version);
						}
						// texture load must be requested
						else {
							loadTex();
						}
					}
					else {
						loadTex();
					}
				}
			}
		}
	}

	std::vector<TextureRef2>& getTexLoadingQueue()
	{
		return loadingQueue;
	}

	void reloadAll()
	{
		version += 1;
		textureRefMap.clear();
		nextTexIndex = 2;
	}
private:
	int version{ 0 };	// if version increases, all texture id's with the old version must be reset
	robin_hood::unordered_map<std::string, std::vector<TextureRef2>> textureRefMap;
	int nextTexIndex{ 2 };

	std::vector<TextureRef2> loadingQueue;
};