#include "Font.h"

#include <iostream>
#include <cmath>
#include <imgui.h>

#include "../Core/Engine.h"
#include "../Classes/IO.h"
#include "../Classes/md5.h"
#include "../Core/Debug.h"
#include "../Core/APIManager.h"
#include "../Assets/Texture.h"

#include "../Classes/ZipHelper.h"

#include <freetype/freetype.h>
#include <freetype/ftglyph.h>

namespace GX
{
	std::string Font::ASSET_TYPE = "Font";

	Font* Font::defaultFont = nullptr;

	#define ANSI_CHARS 128

	FontAtlas::~FontAtlas()
	{
		if (texture != nullptr)
		{
			texture->unload();
			delete texture;
		}
		texture = nullptr;
		glyphs.clear();
	}

	Font::Font() : Asset(APIManager::getSingleton()->font_class)
	{
	}

	Font::~Font()
	{
		
	}

	void Font::unload()
	{
		if (isLoaded())
		{
			Asset::unload();

			clearFontAtlases();
		}
	}

	void Font::unloadAll()
	{
		std::vector<Font*> deleteAssets;

		for (auto it = loadedInstances.begin(); it != loadedInstances.end(); ++it)
		{
			if (it->second->getAssetType() == Font::ASSET_TYPE)
			{
				if (!it->second->getPersistent())
					deleteAssets.push_back((Font*)it->second);
			}
		}

		for (auto it = deleteAssets.begin(); it != deleteAssets.end(); ++it)
			delete (*it);

		deleteAssets.clear();

		defaultFont = nullptr;
	}

	void Font::reload()
	{
		if (getOrigin().empty())
			return;

		if (isLoaded())
			unload();

		load(location, name);
	}

	void Font::load()
	{
		if (!isLoaded())
		{
			Asset::load();
		}
	}

	Font* Font::load(std::string location, std::string name)
	{
		std::string fullPath = location + name;

		Asset* cachedAsset = getLoadedInstance(location, name);

		if (cachedAsset != nullptr && cachedAsset->isLoaded())
		{
			return (Font*)cachedAsset;
		}
		else
		{
			Font* font = nullptr;
			if (cachedAsset == nullptr)
			{
				font = new Font();
				font->setLocation(location);
				font->setName(name);
			}
			else
				font = (Font*)cachedAsset;

			if (IO::isDir(location))
			{
				if (IO::FileExists(fullPath))
					font->load();
				else
					Debug::logWarning("[" + fullPath + "] Error loading font: file does not exists");
			}
			else
			{
				zip_t* arch = Engine::getSingleton()->getZipArchive(location);
				if (ZipHelper::isFileInZip(arch, name))
					font->load();
				else
					Debug::logWarning("[" + fullPath + "] Error loading font: file does not exists");
			}

			return font;
		}
	}

	FontAtlas* Font::getFontAtlas(float resolution, std::function<void(void* data, int size, Texture* tex)> dataCallback)
	{
		float _size = std::min(resolution, 128.0f);

		std::string fullPath = location + name;

		if (IO::isDir(location))
		{
			if (!IO::FileExists(fullPath))
				return nullptr;
		}
		else
		{
			zip_t* arch = Engine::getSingleton()->getZipArchive(location);
			if (!ZipHelper::isFileInZip(arch, name))
				return nullptr;
		}

		auto fnt = std::find_if(fontAtlases.begin(), fontAtlases.end(), [=](FontAtlas* atlas) -> bool
			{
				return atlas->size == _size;
			}
		);

		if (fnt != fontAtlases.end())
			return *fnt;
		
		FontAtlas* fontAtlas = new FontAtlas();
		fontAtlas->size = _size;

		FT_Library lib;
		FT_Init_FreeType(&lib);

		char* faceBuffer = nullptr;

		FT_Face face;
		if (IO::isDir(location))
		{
			FT_New_Face(lib, fullPath.c_str(), 0, &face);
		}
		else
		{
			zip_t* arch = Engine::getSingleton()->getZipArchive(location);

			int sz = 0;
			faceBuffer = ZipHelper::readFileFromZip(arch, name, sz);
			FT_New_Memory_Face(lib, reinterpret_cast<FT_Byte*>(faceBuffer), sz, 0, &face);
		}

		FT_Select_Charmap(face, ft_encoding_unicode);

		FT_Set_Char_Size(face, 0, (int)_size << 6, 96, 96);

		int charCount = 0;
		FT_UInt gindex = 0;
		int charcode = FT_Get_First_Char(face, &gindex);
		while (gindex != 0)
		{
			++charCount;
			charcode = FT_Get_Next_Char(face, charcode, &gindex);
		}

		// quick and dirty max texture size estimate
		int max_dim = (1 + (face->size->metrics.height >> 6)) * std::ceil(std::sqrt(charCount));
		int tex_width = 1;
		while (tex_width < max_dim) tex_width <<= 1;
		int tex_height = tex_width;

		// render glyphs to atlas
		char* pixels = new char[tex_width * tex_height];
		int pen_x = 0, pen_y = 0;

		float maxSizeW = 0;
		float maxSizeH = 0;
		gindex = 0;
		charcode = FT_Get_First_Char(face, &gindex);

		while (gindex != 0)
		{
			FT_Load_Char(face, charcode, FT_LOAD_RENDER | FT_LOAD_FORCE_AUTOHINT | FT_LOAD_TARGET_LIGHT);

			FT_Bitmap* bmp = &face->glyph->bitmap;

			if (pen_x + bmp->width >= tex_width)
			{
				pen_x = 0;
				pen_y += ((face->size->metrics.height >> 6) + 1);
			}

			for (int row = 0; row < bmp->rows; ++row)
			{
				for (int col = 0; col < bmp->width; ++col)
				{
					int x = pen_x + col;
					int y = pen_y + row;
					pixels[y * tex_width + x] = bmp->buffer[row * bmp->pitch + col];
				}
			}

			if (charcode > 0 && charcode < ANSI_CHARS)
			{
				maxSizeW = std::max(maxSizeW, (float)bmp->width);
				maxSizeH = std::max(maxSizeH, (float)bmp->rows);
				fontAtlas->maxGlyphWidth = maxSizeW;
				fontAtlas->maxGlyphHeight = maxSizeH;
			}

			//Store glyphs info
			GlyphInfo info;
			info.rect = glm::vec4(pen_x, pen_y, pen_x + bmp->width, pen_y + bmp->rows);
			info.size = glm::vec2(bmp->width, bmp->rows);
			info.advance = face->glyph->advance.x >> 6;
			info.xOffset = face->glyph->bitmap_left;
			info.yOffset = face->glyph->bitmap_top;

			fontAtlas->glyphs[charcode] = info;

			pen_x += bmp->width + 1;

			charcode = FT_Get_Next_Char(face, charcode, &gindex);
		}

		FT_Done_Face(face);
		FT_Done_FreeType(lib);

		if (faceBuffer != nullptr)
			delete[] faceBuffer;

		int dataSize = tex_width * tex_height * 4;
		char* png_data = new char[dataSize];
		for (int i = 0; i < (tex_width * tex_height); ++i)
		{
			png_data[i * 4 + 0] |= pixels[i];
			png_data[i * 4 + 1] |= pixels[i];
			png_data[i * 4 + 2] |= pixels[i];
			png_data[i * 4 + 3] = pixels[i];
		}

		std::string _texName = name + "[texture_" + std::to_string(_size) + "px]";
		fontAtlas->texture = (Texture*)Asset::getLoadedInstance(location, _texName);
		if (fontAtlas->texture == nullptr)
		{
			fontAtlas->texture = Texture::create(location, _texName, tex_width, tex_height, 1, Texture::TextureType::Texture2D, bgfx::TextureFormat::BGRA8, (unsigned char*)png_data, dataSize, true);
		}
		else
		{
			if (!fontAtlas->texture->isLoaded())
			{
				delete fontAtlas->texture;
				fontAtlas->texture = Texture::create(location, _texName, tex_width, tex_height, 1, Texture::TextureType::Texture2D, bgfx::TextureFormat::BGRA8, (unsigned char*)png_data, dataSize, true);
			}
		}

		if (dataCallback != nullptr)
			dataCallback(png_data, dataSize, fontAtlas->texture);

		delete[] pixels;
		delete[] png_data;

		fontAtlases.push_back(fontAtlas);

		return fontAtlas;
	}

	void Font::clearFontAtlases()
	{
		for (auto it = fontAtlases.begin(); it != fontAtlases.end(); ++it)
			delete* it;

		fontAtlases.clear();
	}

	Font* Font::getDefaultFont()
	{
		if (defaultFont != nullptr && !defaultFont->isLoaded())
		{
			delete defaultFont;
			defaultFont = nullptr;
		}

		if (defaultFont == nullptr)
			defaultFont = Font::load(Engine::getSingleton()->getBuiltinResourcesPath(), "Fonts/arial.ttf");

		if (defaultFont != nullptr && defaultFont->isLoaded())
			return defaultFont;
		else
			return nullptr;
	}
}