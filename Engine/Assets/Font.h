#pragma once

#include <string>
#include <vector>
#include <functional>

#include "Asset.h"

#include "../glm/vec2.hpp"
#include "../glm/vec4.hpp"

namespace GX
{
	class Texture;

	struct GlyphInfo
	{
	public:
		glm::vec4 rect = glm::vec4(0, 0, 0, 0);
		glm::vec2 size = glm::vec2(0, 0);
		float xOffset = 0;
		float yOffset = 0;
		float advance = 0;
	};

	struct FontAtlas
	{
	public:
		~FontAtlas();

		float size = 0;
		Texture* texture = nullptr;
		std::map<int, GlyphInfo> glyphs;
		float maxGlyphWidth = 0;
		float maxGlyphHeight = 0;
	};

	class Font : public Asset
	{
	private:
		std::vector<FontAtlas*> fontAtlases;

		static Font* defaultFont;

	public:
		Font();
		virtual ~Font();

		static std::string ASSET_TYPE;

		virtual void load();
		virtual void unload();
		static void unloadAll();
		virtual void reload();
		virtual std::string getAssetType() { return ASSET_TYPE; }

		static Font* load(std::string location, std::string name);

		FontAtlas* getFontAtlas(float resolution, std::function<void(void* data, int size, Texture* tex)> dataCallback = nullptr);
		void clearFontAtlases();

		static Font* getDefaultFont();
	};
}