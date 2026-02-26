#pragma once

#include <string>
#include <functional>

#include "Asset.h"

#include <bgfx/bgfx.h>

#include "../Math/Rect.h"
#include "../Renderer/Color.h"

#undef None

struct FIBITMAP;

namespace GX
{
	class Texture : public Asset
	{
		friend class Renderer;
		friend class Asset;
		friend class RenderTexture;
		friend class Camera;

	public:
		enum class TextureType
		{
			Texture2D,
			Texture3D,
			Cubemap
		};

		enum class CompressionMethod
		{
			Default,
			None,
			BC7
		};

		enum class WrapMode
		{
			Repeat,
			Clamp
		};

		enum class FilterMode
		{
			Point,
			Linear,
			Anisotropic,
		};

	private:
		int width = 0;
		int height = 0;
		int originalWidth = 0;
		int originalHeight = 0;
		int bpp = 0;
		int numMipMaps = 0;
		int size = 0;
		bool genMipMaps = true;
		int maxResolution = 0;
		bool keepData = false;
		FilterMode filterMode = FilterMode::Anisotropic;
		WrapMode wrapMode = WrapMode::Repeat;
		CompressionMethod compressionMethod = CompressionMethod::Default;
		int compressionQuality = 0; //Higher - better but slower. 0 = default
		Rect border = Rect(0, 0, 0, 0);

		uint64_t getState();

		unsigned char * pixels = nullptr;
		bgfx::TextureHandle textureHandle = { bgfx::kInvalidHandle };

		static Texture* nullTexture;
		bool immutable = true;

		const bgfx::Memory* mem = nullptr;
		bgfx::TextureFormat::Enum format = bgfx::TextureFormat::BC7;

		TextureType textureType = TextureType::Texture2D;
		int generateMipMaps(FIBITMAP * bitmap);

		static void releaseDataCallback(void* _ptr, void* _userData);
		void updateTextureCb(bool cb);

	public:
		Texture();
		virtual ~Texture();

		static std::string ASSET_TYPE;

		virtual void load();
		virtual void unload();
		static void unloadAll();
		virtual void reload();
		virtual std::string getAssetType() { return ASSET_TYPE; }

		static Texture* load(std::string location,
			std::string name,
			bool genMipMaps = true,
			CompressionMethod compression = CompressionMethod::Default,
			bool setIsPersistent = false,
			bool warn = true,
			std::function<void(unsigned char* data, size_t size)> cb = nullptr);

		static Texture* loadFromByteArray(std::string location, std::string name, unsigned char* data, size_t size);
		static Texture* create(std::string location, std::string name, int w, int h, int numLayers, TextureType type, bgfx::TextureFormat::Enum format = bgfx::TextureFormat::BGRA8, unsigned char* data = nullptr, size_t size = 0, bool genMipMaps = false, bool keepTexData = false);
		void save();
		void save(std::string path);
		static Texture* getNullTexture() { return nullTexture; }
		void allocData(int dataSize);
		void freeData();
		unsigned char*& getData() { return pixels; }
		void updateTexture();

		bgfx::TextureHandle getHandle() { return textureHandle; }
		TextureType getTextureType() { return textureType; }

		static void createNullTexture();
		static void destroyNullTexture();

		std::string getCachedFileName();

		WrapMode getWrapMode() { return wrapMode; }
		void setWrapMode(WrapMode value);

		bool getGenMipMaps() { return genMipMaps; }
		void setGenMipMaps(bool value);

		FilterMode getFilterMode() { return filterMode; }
		void setFilterMode(FilterMode value);

		CompressionMethod getCompressionMethod() { return compressionMethod; }
		void setCompressionMethod(CompressionMethod value);

		int getCompressionQuality() { return compressionQuality; }
		void setCompressionQuality(int value);

		int getMaxResolution() { return maxResolution; }
		void setMaxResolution(int value);

		bgfx::TextureFormat::Enum getFormat() { return format; }

		Color getPixel(int x, int y);
		void setPixel(int x, int y, Color color);

		int getWidth() { return width; }
		int getHeight() { return height; }
		int getOriginalWidth() { return originalWidth; }
		int getOriginalHeight() { return originalHeight; }
		int getBpp() { return bpp; }
		int getNumMipMaps() { return numMipMaps; }
		int getSize() { return size; }
		bool getImmutable() { return immutable; }
		Rect getBorder() { return border; }
		void setBorder(Rect value) { border = value; }
		const bgfx::Memory* getMemory() { return mem; }
	};
}