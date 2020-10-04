#pragma once

#include <boost/serialization/access.hpp>
#include <boost/serialization/unordered_map.hpp>
#include <boost/serialization/vector.hpp>
#include <vector>
#include <string>
#include <map>

class TextureUniforms {
public:
	static constexpr int WHITE = 0;
	static constexpr int DEFAULT = 1; 
	TextureUniforms()
	{
		textureNames.push_back(std::make_unique<std::string>("white"));
		textureNames.push_back(std::make_unique<std::string>("default"));
		textureNameToId["white"] = WHITE;
		textureNameToId["default"] = DEFAULT;
	}
	int getId(const std::string& filename);
	const std::string& getTextureName(int textureId) const;
	const std::string& getTextureName(const SmallTextureRef& texRef) const;
private:
	friend class boost::serialization::access;
	template<class Archive>
	void save(Archive& ar, const unsigned int version) const
	{
		std::vector<std::string> sv;
		for (const auto& s : textureNames) {
			sv.emplace_back(std::string(*s));
		}
		printf("sv size save: %i\n", sv.size());
		ar << sv;
		ar << textureNameToId;
	}

	template<class Archive>
	void load(Archive& ar, const unsigned int version)
	{
		textureNames.clear();
		std::vector<std::string> sv;
		ar >> sv;
		printf("\n\nsv size load: %i\n\n\n", sv.size());
		for (auto& s : sv) {
			textureNames.push_back(std::make_unique<std::string>(std::move(s)));
		}

		textureNameToId.clear();
		ar >> textureNameToId;
	}

	template<class Archive>
	void serialize(Archive& ar, const unsigned int file_version)
	{
		boost::serialization::split_member(ar, *this, file_version);
	}
	friend class Renderer;
	std::unordered_map<std::string, int> textureNameToId;
	std::vector<std::unique_ptr<std::string>> textureNames;
};

inline int TextureUniforms::getId(const std::string& filename)
{
	auto iter = textureNameToId.find(filename);
	if (iter == textureNameToId.end()) {
		textureNames.push_back(std::make_unique<std::string>(filename));
		textureNameToId[filename] = (int)textureNames.size() - 1;
	}
	return textureNameToId[filename];
}

inline const std::string& TextureUniforms::getTextureName(int textureIndex) const
{
	if (textureIndex < 0 || textureIndex >= textureNames.size()) {
		std::cerr << "ERROR: The texture index: " << textureIndex << " has no correcponding texture!" << std::endl;
		exit(-1);
	}
	return *textureNames[textureIndex];
}

inline const std::string& TextureUniforms::getTextureName(const SmallTextureRef& texRef) const
{
	if (texRef.textureId < 0 || texRef.textureId >= textureNames.size()) {
		std::cerr << "ERROR: The texture index: " << texRef.textureId << " has no correcponding texture!" << std::endl;
		exit(-1);
	}
	return *textureNames[texRef.textureId];
}