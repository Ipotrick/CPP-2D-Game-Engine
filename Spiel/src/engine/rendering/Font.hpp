#pragma once

#include <string>
#include <fstream>
#include <sstream>

#include "../types/PagedIndexMap.hpp"

#include "pipeline/RessourceManager.hpp"
#include "OpenGLAbstraction/OpenGLTexture.hpp"

#include "Glyph.hpp"

struct FontDescriptor {
	std::string atlasFilePath;

	bool operator==(const FontDescriptor& rhs) const
	{
		return atlasFilePath == rhs.atlasFilePath;
	}

	bool holdsValue() const { return atlasFilePath.size() > 0; }
};

namespace std {
	template<> struct hash<FontDescriptor> {
		std::size_t operator()(FontDescriptor const& d) const noexcept
		{
			return std::hash<std::string>{}(d.atlasFilePath);
		}
	};
}

struct Font {
	Font() = default;
	Font(Font&& rhs) = default;
	Font(const FontDescriptor& d);
	~Font();
	void load(const FontDescriptor& d);
	void unload();

	FontDescriptor desc;
	PagedIndexMap<Glyph> codepointToGlyph;
private:
	static std::pair<u32, Glyph> loadGlyphFromCSVLine(std::stringstream& buffer);
};

struct FontHandle : public RessourceHandleBase {};

using FontManager = RessourceManager<FontHandle, FontDescriptor, Font>;
