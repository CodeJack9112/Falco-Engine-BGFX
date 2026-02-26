#include "Texture.h"

#include <boost/iostreams/stream.hpp>

#include <iostream>
#include <cmath>
#include <bx/bx.h>
#include "Classes/bc7compressor.h"

#ifndef FREEIMAGE_LIB
#define FREEIMAGE_LIB
#include "../FreeImage/include/FreeImage.h"
#undef FREEIMAGE_LIB
#endif

#include "../Core/Engine.h"
#include "../Renderer/Renderer.h"
#include "../Classes/IO.h"
#include "../Classes/md5.h"
#include "../Core/Debug.h"
#include "../Core/APIManager.h"
#include "../Renderer/NullTextureData.h"
#include "Cubemap.h"

#include "Serialization/Assets/STexture.h"
#include "../Classes/ZipHelper.h"

namespace GX
{
	std::string Texture::ASSET_TYPE = "Texture";
	Texture* Texture::nullTexture = nullptr;

	Texture::Texture() : Asset(APIManager::getSingleton()->texture_class)
	{
	}

	Texture::~Texture()
	{
		if (!persistent)
		{
			if (!immutable)
			{
				updateTextureCb(true);
			}
		}

		unload();
	}

	void Texture::releaseDataCallback(void* _ptr, void* _userData)
	{
		if (_userData != nullptr)
		{
			Texture* texture = reinterpret_cast<Texture*>(_userData);

			if (!texture->keepData)
			{
				if (_ptr != nullptr)
					delete[] _ptr;

				texture->pixels = nullptr;
				texture->mem = nullptr;
			}
		}
		else
		{
			if (_ptr != nullptr)
				delete[] _ptr;
		}
	}

	void Texture::unload()
	{
		if (isLoaded())
		{
			Asset::unload();

			if (persistent)
				return;

			if (bgfx::isValid(textureHandle))
				bgfx::destroy(textureHandle);

			if (immutable && !persistent)
			{
				if (pixels != nullptr)
					delete[] pixels;
			}

			pixels = nullptr;

			textureHandle = { bgfx::kInvalidHandle };

			//if (!immutable)
			Renderer::getSingleton()->frame();
		}
	}

	void Texture::unloadAll()
	{
		std::vector<Texture*> deleteAssets;

		for (auto it = loadedInstances.begin(); it != loadedInstances.end(); ++it)
		{
			if (it->second->getAssetType() == Texture::ASSET_TYPE)
			{
				if (!it->second->getPersistent())
					deleteAssets.push_back((Texture*)it->second);
			}
		}

		for (auto it = deleteAssets.begin(); it != deleteAssets.end(); ++it)
			delete (*it);

		deleteAssets.clear();
	}

	void Texture::reload()
	{
		if (persistent)
			return;

		if (!immutable)
			return;

		if (getOrigin().empty())
			return;

		if (isLoaded())
			unload();

		std::string texName = getCachedFileName();

		if (!Engine::getSingleton()->getIsRuntimeMode())
		{
			if (IO::FileExists(texName))
				IO::FileDelete(texName);
		}

		load(location, name, genMipMaps, compressionMethod, persistent);

		for (auto& asset : Asset::getLoadedInstances())
		{
			if (asset.second->getAssetType() == Cubemap::ASSET_TYPE)
			{
				Cubemap* cubemap = (Cubemap*)asset.second;

				for (int i = 0; i < 6; ++i)
				{
					if (cubemap->getTexture(i) == this)
					{
						cubemap->reload();
						break;
					}
				}
			}
		}
	}

	void Texture::load()
	{
		if (!isLoaded())
		{
			Asset::load();
		}

		Renderer::getSingleton()->frame();
	}

	Texture* Texture::loadFromByteArray(std::string location, std::string name, unsigned char* data, size_t size)
	{
		Texture* texture = nullptr;

		Asset* cachedTexture = Asset::getLoadedInstance(location, name);
		if (cachedTexture != nullptr)
		{
			if (cachedTexture->isLoaded())
			{
				texture = (Texture*)cachedTexture;
				return texture;
			}
			else
				texture = (Texture*)cachedTexture;
		}
		else
			texture = new Texture();

		texture->setName(name);
		texture->setLocation(location);

		FIMEMORY* fimem = FreeImage_OpenMemory(data, size);
		FREE_IMAGE_FORMAT formato = FreeImage_GetFileTypeFromMemory(fimem, size);
		FIBITMAP* imagen = FreeImage_LoadFromMemory(formato, fimem);
		imagen = FreeImage_ConvertToRGBA16(imagen);

		texture->width = FreeImage_GetWidth(imagen);
		texture->height = FreeImage_GetHeight(imagen);
		texture->originalWidth = texture->width;
		texture->originalHeight = texture->height;
		texture->bpp = FreeImage_GetBPP(imagen);

		int _size = (texture->bpp / 8) * texture->width * texture->height;

		//delete[] nullTexture->pixels;
		unsigned char* _data = FreeImage_GetBits(imagen);
		texture->pixels = new unsigned char[_size];
		memcpy(texture->pixels, _data, _size);
		_data = nullptr;

		texture->size = _size;

		texture->format = bgfx::TextureFormat::RGBA16;

		const bgfx::Memory* mem = bgfx::makeRef(reinterpret_cast<const void*>(texture->pixels), _size);

		//bgfx::destroy(nullTexture->textureHandle);
		texture->textureHandle = bgfx::createTexture2D(uint16_t(texture->width), uint16_t(texture->height), false, 1, bgfx::TextureFormat::RGBA16,
			BGFX_SAMPLER_MIN_ANISOTROPIC
			| BGFX_SAMPLER_MAG_ANISOTROPIC
			| BGFX_SAMPLER_MIP_POINT, mem);

		FreeImage_Unload(imagen);
		FreeImage_CloseMemory(fimem);

		texture->load();

		return texture;
	}

	void copyPixels(color_quad_u8_vec& dst, FIBITMAP* src, int width, int height)
	{
		BYTE* pixels = (BYTE*)FreeImage_GetBits(src);

		FIBITMAP* alphaChannel = FreeImage_GetChannel(src, FREE_IMAGE_COLOR_CHANNEL::FICC_ALPHA);

		BYTE* bits = FreeImage_GetBits(alphaChannel);

		int pixelsCount = width * height;
		dst.resize(pixelsCount);

		int pos = 0;
		for (int i = 0; i < width; ++i)
		{
			for (int j = 0; j < height; ++j)
			{
				color_quad_u8 pixel;
				RGBQUAD rgb;
				FreeImage_GetPixelColor(src, j, i, &rgb);
				
				BYTE alpha = 255;
				if (alphaChannel != nullptr)
					alpha = pixels[pos * 4 + 3];
				
				pixel.set(rgb.rgbRed, rgb.rgbGreen, rgb.rgbBlue, alpha);
				dst[pos] = pixel;
				++pos;
			}
		}

		if (alphaChannel != nullptr)
			FreeImage_Unload(alphaChannel);
	}

	uint64_t Texture::getState()
	{
		uint64_t state_aniso = BGFX_SAMPLER_MIN_ANISOTROPIC
			| BGFX_SAMPLER_MAG_ANISOTROPIC;

		uint64_t state_point = BGFX_SAMPLER_MIN_POINT
			| BGFX_SAMPLER_MAG_POINT;

		uint64_t texState = 0;

		if (filterMode == FilterMode::Point)
			texState = state_point;

		if (filterMode == FilterMode::Anisotropic)
			texState = state_aniso;

		if (wrapMode == WrapMode::Clamp)
			texState |= BGFX_SAMPLER_UVW_CLAMP;

		return texState;
	}

	Texture* Texture::load(std::string location, std::string name, bool genMipMaps, CompressionMethod compression, bool setIsPersistent, bool warn, std::function<void(unsigned char* data, size_t size)> cb)
	{
		if (location.empty() || name.empty())
			return nullptr;

		ProjectSettings* settings = Engine::getSingleton()->getSettings();

		std::string fullPath = location + name;

		std::string libLocation = Engine::getSingleton()->getLibraryPath();
		std::string texDir = IO::GetFilePath(name);
		std::string texName = libLocation + texDir + md5(name) + ".texture";

		bool checkSourceFile = true;
		//if (compression != CompressionMethod::None)
		if (!setIsPersistent)
		{
			if (IO::isDir(libLocation))
			{
				if (!IO::FileExists(texName))
				{
					if (IO::FileExists(fullPath))
					{
						if (!IO::DirExists(libLocation + texDir))
							IO::CreateDir(libLocation + texDir, true);
					}
				}
				else
					checkSourceFile = false;
			}
		}

		Asset * cachedAsset = getLoadedInstance(location, name);

		if (cachedAsset != nullptr && cachedAsset->isLoaded())
		{
			Texture* tex = (Texture*)cachedAsset;
			tex->setPersistent(setIsPersistent);

			return tex;
		}
		else
		{
			if (checkSourceFile)
			{
				if (IO::isDir(libLocation) || libLocation.empty())
				{
					if (!IO::FileExists(fullPath))
					{
						if (warn)
							Debug::logWarning("[" + fullPath + "] Error loading texture: file does not exists");

						return nullptr;
					}
				}
				else
				{
					std::string _tname = IO::RemovePart(texName, libLocation);
					zip_t* arch = Engine::getSingleton()->getZipArchive(libLocation);
					if (!ZipHelper::isFileInZip(arch, _tname))
					{
						if (warn)
							Debug::logWarning("[" + fullPath + "] Error loading texture: file does not exists");

						return nullptr;
					}
				}
			}

			Texture* texture = nullptr;
			if (cachedAsset == nullptr)
			{
				texture = new Texture();
				texture->setLocation(location);
				texture->setName(name);
				texture->setAlias(name);
			}
			else
				texture = (Texture*)cachedAsset;

			texture->setPersistent(setIsPersistent);
			texture->genMipMaps = genMipMaps;
			texture->compressionMethod = compression;

			//int size = 0;

			STexture sTexture;
			bool loadedFromCache = false;

			if (IO::isDir(libLocation) || libLocation.empty())
			{
				if (IO::FileExists(texName))
				{
					//Load from cache in Library folder
					try
					{
						std::ifstream ofs(texName, std::ios::binary);
						BinarySerializer s;
						s.deserialize(&ofs, &sTexture, Texture::ASSET_TYPE);
						ofs.close();
					}
					catch (std::exception e)
					{
						Debug::log("[" + name + "] Error loading texture cache: " + e.what());
					}

					loadedFromCache = true;
				}
			}
			else
			{
				std::string _tname = IO::RemovePart(texName, libLocation);
				zip_t* arch = Engine::getSingleton()->getZipArchive(libLocation);

				int sz = 0;
				char* buffer = ZipHelper::readFileFromZip(arch, _tname, sz);

				try
				{
					boost::iostreams::stream<boost::iostreams::array_source> is(buffer, sz);
					BinarySerializer s;
					s.deserialize(&is, &sTexture, Texture::ASSET_TYPE);
					is.close();
				}
				catch (std::exception e)
				{
					Debug::log("[" + name + "] Error loading texture cache: " + e.what());
				}

				delete[] buffer;
				loadedFromCache = true;
			}

			if (loadedFromCache && !setIsPersistent/* && compression != CompressionMethod::None*/)
			{
				//size = sTexture.size;

				texture->width = sTexture.width;
				texture->height = sTexture.height;
				texture->originalWidth = sTexture.originalWidth;
				texture->originalHeight = sTexture.originalHeight;
				texture->bpp = sTexture.bpp;
				texture->numMipMaps = sTexture.numMipMaps;
				texture->pixels = new unsigned char[sTexture.size];
				texture->size = sTexture.size;
				texture->compressionMethod = static_cast<CompressionMethod>(sTexture.compressionMethod);
				texture->compressionQuality = sTexture.compressionQuality;
				texture->wrapMode = static_cast<WrapMode>(sTexture.wrapMode);
				texture->filterMode = static_cast<FilterMode>(sTexture.filterMode);
				texture->genMipMaps = sTexture.genMipMaps;
				texture->maxResolution = sTexture.maxResolution;
				texture->border = sTexture.border.getValue();
				if (sTexture.pixels.size() > 0)
					memcpy(texture->pixels, &sTexture.pixels[0], sTexture.size);
				sTexture.pixels.clear();
			}
			else
			{
				//Load from source and create cache
				FREE_IMAGE_FORMAT formato = FreeImage_GetFileType(fullPath.c_str(), 0);
				FIBITMAP* imagen = FreeImage_Load(formato, fullPath.c_str());
				if (imagen == nullptr)
					return nullptr;

				if (FreeImage_GetBPP(imagen) != 32)
				{
					FIBITMAP* convert = FreeImage_ConvertTo32Bits(imagen);
					FreeImage_Unload(imagen);
					imagen = convert;
				}

				int maxRes = texture->maxResolution;
				if (maxRes == 0)
					maxRes = settings->getTextureMaxResolution();
				if (texture->persistent)
					maxRes = 4096;

				int w = FreeImage_GetWidth(imagen);
				int h = FreeImage_GetHeight(imagen);
				
				size_t u2 = 1; while (u2 < w) u2 *= 2;
				size_t v2 = 1; while (v2 < h) v2 *= 2;

				if (u2 > 1)
				{
					int pw = u2 / 2;
					if (w - pw < u2 - w)
						u2 = pw;
				}

				if (v2 > 1)
				{
					int ph = v2 / 2;
					if (h - ph < v2 - h)
						v2 = ph;
				}

				int w2 = std::min((int)u2, maxRes);
				int h2 = std::min((int)v2, maxRes);

				texture->originalWidth = w;
				texture->originalHeight = h;

				if (w != w2 || h != h2 || w != h)
				{
					int s = std::max(w2, h2);

					FIBITMAP* scaled = FreeImage_Rescale(imagen, s, s, FREE_IMAGE_FILTER::FILTER_BOX);
					FreeImage_Unload(imagen);
					imagen = scaled;
				}

				texture->width = FreeImage_GetWidth(imagen);
				texture->height = FreeImage_GetHeight(imagen);
				texture->bpp = FreeImage_GetBPP(imagen);

				int size = 0;

				CompressionMethod _compression = texture->compressionMethod;
				if (_compression == CompressionMethod::Default)
					_compression = static_cast<CompressionMethod>(settings->getTextureCompression());

				int _compressionQuality = texture->compressionQuality - 1;
				if (_compressionQuality == -1)
					_compressionQuality = settings->getTextureCompressionQuality();

				if (texture->genMipMaps)
				{
					if (_compression == CompressionMethod::None)
					{
						FIBITMAP* _imagen = FreeImage_ConvertToRGBA16(imagen);
						FreeImage_Unload(imagen);
						imagen = _imagen;
						texture->bpp = FreeImage_GetBPP(imagen);
					}

					size = texture->generateMipMaps(imagen);
				}
				else
				{
					if (_compression == CompressionMethod::BC7)
					{
						color_quad_u8_vec pixels;
						uint32_t imageSize = (((texture->width + 3) & ~3) * ((texture->height + 3) & ~3) * 8) >> 3;
						texture->pixels = new unsigned char[imageSize];
						copyPixels(pixels, imagen, texture->width, texture->height);
						bc7compress(pixels, texture->width, texture->height, texture->pixels, size, _compressionQuality);
						pixels.clear();
					}
					else
					{
						FIBITMAP* _imagen = FreeImage_ConvertToRGBA16(imagen);
						FreeImage_Unload(imagen);
						imagen = _imagen;
						texture->bpp = FreeImage_GetBPP(imagen);

						size = (texture->bpp / 8) * (texture->width * texture->height);
						texture->pixels = new unsigned char[size];
						memcpy(texture->pixels, FreeImage_GetBits(imagen), size);
					}
				}

				texture->size = size;

				FreeImage_Unload(imagen);

				//if (compression != CompressionMethod::None)
				if (!setIsPersistent)
					texture->save(texName);
			}

			CompressionMethod _compression = texture->compressionMethod;
			if (_compression == CompressionMethod::Default)
				_compression = static_cast<CompressionMethod>(settings->getTextureCompression());

			if (_compression == CompressionMethod::None)
				texture->format = bgfx::TextureFormat::RGBA16;
			else if (_compression == CompressionMethod::BC7)
				texture->format = bgfx::TextureFormat::BC7;

			texture->mem = bgfx::makeRef(reinterpret_cast<const void*>(texture->pixels), texture->size, releaseDataCallback, reinterpret_cast<void*>(texture));
			texture->textureHandle = bgfx::createTexture2D(uint16_t(texture->width), uint16_t(texture->height), texture->genMipMaps, 1, texture->format, texture->getState(), texture->mem);
			
			if (cb != nullptr)
				cb(texture->pixels, texture->size);

			texture->load();

			return texture;
		}
	}

	Texture* Texture::create(std::string location, std::string name, int w, int h, int numLayers, TextureType type, bgfx::TextureFormat::Enum format, unsigned char* data, size_t size, bool genMipMaps, bool keepTexData)
	{
		Asset* cachedAsset = getLoadedInstance(location, name);
		assert(cachedAsset == nullptr && "Asset with these location and name already exists");

		Texture* texture = new Texture();
			
		texture->setLocation(location);
		texture->setName(name);
		texture->width = w;
		texture->height = h;
		texture->originalWidth = w;
		texture->originalHeight = h;
		texture->textureType = type;
		texture->format = format;
		texture->keepData = keepTexData;

		if (type == TextureType::Texture2D)
		{
			if (data == nullptr || size == 0) //Dynamic texture
			{
				texture->immutable = false;
				texture->textureHandle = bgfx::createTexture2D(uint16_t(texture->width), uint16_t(texture->height), false, 1, texture->format,
					BGFX_SAMPLER_MIN_ANISOTROPIC
					| BGFX_SAMPLER_MAG_ANISOTROPIC);
			}
			else //Immutable texture
			{
				if (genMipMaps)
				{
					FIBITMAP* imagen = FreeImage_ConvertFromRawBits(data, w, h, w * 4, 32, 0xFF0000, 0x00FF00, 0x0000FF, false);

					texture->bpp = 32;
					texture->compressionMethod = Texture::CompressionMethod::None;

					int sz = texture->generateMipMaps(imagen);
					texture->size = sz;

					FreeImage_Unload(imagen);
				}
				else
				{
					texture->size = size;
					texture->pixels = new unsigned char[size];
					memcpy(texture->pixels, data, size);
				}

				texture->immutable = true;
				texture->mem = bgfx::makeRef(reinterpret_cast<const void*>(texture->pixels), texture->size, releaseDataCallback, reinterpret_cast<void*>(texture));

				texture->textureHandle = bgfx::createTexture2D(uint16_t(texture->width), uint16_t(texture->height), genMipMaps, 1, texture->format,
					BGFX_SAMPLER_MIN_ANISOTROPIC
					| BGFX_SAMPLER_MAG_ANISOTROPIC, texture->mem);
			}

			texture->load();
		}

		return texture;
	}

	void Texture::createNullTexture()
	{
		if (nullTexture != nullptr)
			return;

		nullTexture = Texture::loadFromByteArray("system/", "NullTexture", nullTextureData, sizeof(nullTextureData) / sizeof(char));
		nullTexture->setPersistent(true);
	}

	void Texture::destroyNullTexture()
	{
		if (nullTexture != nullptr)
		{
			if (Asset::getLoadedInstance("system/", "NullTexture") != nullptr)
			{
				delete nullTexture->getData();
				delete nullTexture;
			}

			nullTexture = nullptr;
		}
	}

	std::string Texture::getCachedFileName()
	{
		std::string libLocation = Engine::getSingleton()->getLibraryPath();
		std::string texDir = IO::GetFilePath(name);
		std::string texName = libLocation + texDir + md5(name) + ".texture";

		return texName;
	}

	void Texture::save()
	{
		std::string texName = getCachedFileName();

		save(texName);
	}

	void Texture::save(std::string path)
	{
		assert(pixels != nullptr && "Texture is empty!");

		STexture sTexture;
		sTexture.width = width;
		sTexture.height = height;
		sTexture.originalWidth = originalWidth;
		sTexture.originalHeight = originalHeight;
		sTexture.bpp = bpp;
		sTexture.numMipMaps = numMipMaps;
		sTexture.size = size;
		sTexture.compressionMethod = static_cast<int>(compressionMethod);
		sTexture.compressionQuality = static_cast<int>(compressionQuality);
		sTexture.wrapMode = static_cast<int>(wrapMode);
		sTexture.filterMode = static_cast<int>(filterMode);
		sTexture.genMipMaps = genMipMaps;
		sTexture.maxResolution = maxResolution;
		sTexture.border = border;
		sTexture.pixels.resize(size);
		memcpy(&sTexture.pixels[0], pixels, size);

		std::ofstream ofs(path, std::ios::binary);
		
		BinarySerializer s;
		s.serialize(&ofs, &sTexture, Texture::ASSET_TYPE);
		ofs.close();

		sTexture.pixels.clear();
	}

	void Texture::allocData(int dataSize)
	{
		//assert(pixels == nullptr && "Data must be null");

		assert(!immutable && "Texture must be not immutable");
		if (pixels != nullptr)
			delete[] pixels;

		pixels = new unsigned char[dataSize];
		size = dataSize;
	}

	void Texture::freeData()
	{
		if (pixels != nullptr)
			delete[] pixels;

		pixels = nullptr;
	}

	void Texture::updateTexture()
	{
		if (immutable)
			return;

		updateTextureCb(false);
	}

	void Texture::updateTextureCb(bool cb)
	{
		if (cb)
			mem = bgfx::makeRef(reinterpret_cast<const void*>(pixels), size, releaseDataCallback, reinterpret_cast<void*>(this));
		else
			mem = bgfx::makeRef(reinterpret_cast<const void*>(pixels), size);

		bgfx::updateTexture2D(textureHandle, 0, 0, 0, 0, width, height, mem);
	}

	void Texture::setWrapMode(WrapMode value)
	{
		wrapMode = value;
		reload();
	}

	void Texture::setGenMipMaps(bool value)
	{
		genMipMaps = value;
		reload();
	}

	void Texture::setFilterMode(FilterMode value)
	{
		filterMode = value;
		reload();
	}

	void Texture::setCompressionMethod(CompressionMethod value)
	{
		compressionMethod = value;
		reload();
	}

	void Texture::setCompressionQuality(int value)
	{
		compressionQuality = value;
		reload();
	}

	void Texture::setMaxResolution(int value)
	{
		maxResolution = value;
		reload();
	}

	Color Texture::getPixel(int x, int y)
	{
		Color color;

		if (compressionMethod == CompressionMethod::None)
		{
			if (pixels != nullptr)
			{
				if (x >= 0 && y >= 0 && x < width && y < height)
				{
					color[0] = pixels[(x * width + y) * 4];
					color[1] = pixels[(x * width + y) * 4 + 1];
					color[2] = pixels[(x * width + y) * 4 + 2];
					color[3] = pixels[(x * width + y) * 4 + 3];
				}
			}
		}
		return color;
	}

	void Texture::setPixel(int x, int y, Color color)
	{
		if (pixels != nullptr)
		{
			if (compressionMethod == CompressionMethod::None)
			{
				if (x >= 0 && y >= 0 && x < width && y < height)
				{
					pixels[(x * width + y) * 4] = color.r();
					pixels[(x * width + y) * 4 + 1] = color.g();
					pixels[(x * width + y) * 4 + 2] = color.b();
					pixels[(x * width + y) * 4 + 3] = color.a();
				}
			}
		}
	}

	int Texture::generateMipMaps(FIBITMAP* bitmap)
	{
		int width = FreeImage_GetWidth(bitmap);
		int height = FreeImage_GetHeight(bitmap);
		int bpp = FreeImage_GetBPP(bitmap);

		int size0 = (bpp / 8) * width * height;

		ProjectSettings* settings = Engine::getSingleton()->getSettings();

		CompressionMethod _compression = compressionMethod;
		if (_compression == CompressionMethod::Default)
			_compression = static_cast<CompressionMethod>(settings->getTextureCompression());

		int _compressionQuality = compressionQuality - 1;
		if (_compressionQuality == -1)
			_compressionQuality = settings->getTextureCompressionQuality();

		if (_compression == CompressionMethod::BC7)
			size0 = (((width + 3) & ~3) * ((height + 3) & ~3) * 8) >> 3;
		
		int size = size0;

		int numMips = static_cast<uint32_t>(std::floor(std::log2(std::max(width, height)))) + 1;

		numMipMaps = numMips;

		int div = 2;
		for (int i = 1; i < numMips; ++i)
		{
			int _w = floor(width / div);
			int _h = floor(height / div);

			if (_compression == CompressionMethod::BC7)
				size += (((_w + 3) & ~3) * ((_h + 3) & ~3) * 8) >> 3;
			else
				size += (bpp / 8) * (_w * _h);
			
			div *= 2;
		}

		pixels = new unsigned char[size];

		if (_compression == CompressionMethod::BC7)
		{
			int sz0 = 0;
			color_quad_u8_vec dst;
			copyPixels(dst, bitmap, width, height);
			bc7compress(dst, width, height, pixels, sz0, _compressionQuality);
			dst.clear();
		}
		else
			memcpy(pixels, FreeImage_GetBits(bitmap), size0);

		div = 2;
		int offset = size0;

		FIBITMAP* prevScaled = bitmap;
		
		for (int i = 1; i < numMips; ++i)
		{
			int _w = floor(width / div);
			int _h = floor(height / div);

			FIBITMAP* scaled = FreeImage_Rescale(prevScaled, _w, _h, FREE_IMAGE_FILTER::FILTER_BOX);
			if (prevScaled != bitmap)
				FreeImage_Unload(prevScaled);
			prevScaled = scaled;

			int ww = FreeImage_GetWidth(scaled);
			int hh = FreeImage_GetHeight(scaled);
			int bpp = FreeImage_GetBPP(scaled);

			int sz = (bpp / 8) * ww * hh;

			if (_compression == CompressionMethod::BC7)
				sz = (((ww + 3) & ~3) * ((hh + 3) & ~3) * 8) >> 3;

			if (_compression == CompressionMethod::BC7)
			{
				color_quad_u8_vec dst;
				unsigned char* dt = new unsigned char[sz];
				copyPixels(dst, scaled, ww, hh);
				bc7compress(dst, ww, hh, dt, sz, _compressionQuality);
				memcpy(pixels + offset, dt, sz);
				dst.clear();
				delete[] dt;
			}
			else
				memcpy(pixels + offset, FreeImage_GetBits(scaled), sz);

			offset += sz;
			//FreeImage_Unload(scaled);
			div *= 2;
		}

		if (prevScaled != bitmap)
			FreeImage_Unload(prevScaled);

		return size;
	}
}