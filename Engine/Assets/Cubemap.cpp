#include "Cubemap.h"

#include <boost/iostreams/stream.hpp>

#include "../Core/Engine.h"
#include "../Core/APIManager.h"
#include "../Classes/IO.h"
#include "../Core/Debug.h"
#include "../Classes/ZipHelper.h"

#include "Texture.h"

#include "../Serialization/Assets/SCubemap.h"

namespace GX
{
	Cubemap* Cubemap::nullCubemap = nullptr;

	std::string Cubemap::ASSET_TYPE = "Cubemap";

	Cubemap::Cubemap() : Asset(APIManager::getSingleton()->cubemap_class)
	{
		for (int i = 0; i < 6; ++i)
			textures[i] = nullptr;
	}

	Cubemap::~Cubemap()
	{
		unload();
	}

	void Cubemap::releaseDataCallback(void* _ptr, void* _userData)
	{
		delete[] _ptr;
	}

	void Cubemap::unload()
	{
		if (isLoaded())
		{
			Asset::unload();

			if (persistent)
				return;

			if (bgfx::isValid(textureHandle))
				bgfx::destroy(textureHandle);

			textureHandle = { bgfx::kInvalidHandle };

			for (int i = 0; i < 6; ++i)
				textures[i] = nullptr;
		}
	}

	void Cubemap::unloadAll()
	{
		std::vector<Cubemap*> deleteAssets;

		for (auto it = loadedInstances.begin(); it != loadedInstances.end(); ++it)
		{
			if (it->second->getAssetType() == Cubemap::ASSET_TYPE)
			{
				if (!it->second->getPersistent())
					deleteAssets.push_back((Cubemap*)it->second);
			}
		}

		for (auto it = deleteAssets.begin(); it != deleteAssets.end(); ++it)
			delete (*it);

		deleteAssets.clear();
	}

	void Cubemap::reload()
	{
		if (getOrigin().empty())
			return;

		if (isLoaded())
			unload();

		load(location, name);
	}

	void Cubemap::load()
	{
		if (!isLoaded())
		{
			Asset::load();
		}
	}

	void Cubemap::updateCubemap()
	{
		if (bgfx::isValid(textureHandle))
			bgfx::destroy(textureHandle);

		bool valid = true;

		Texture* _textures[6];
		std::pair<unsigned char*, int> datas[6];

		int wIdx = 0;
		for (int i = 0; i < 6; ++i)
		{
			if (textures[i] == nullptr || !textures[i]->isLoaded())
			{
				_textures[i] = Texture::getNullTexture();
				datas[i].first = new unsigned char[_textures[i]->getSize()];
				datas[i].second = _textures[i]->getSize();
				memcpy(datas[i].first, _textures[i]->getData(), _textures[i]->getSize());
			}
			else
			{
				_textures[i] = textures[i];

				if (_textures[i]->getData() == nullptr)
				{
					//Need to load data from disc again, as it moved to GPU memory and not available until reload
					_textures[i]->unload();
					_textures[i] = Texture::load(_textures[i]->getLocation(), _textures[i]->getName(), true, Texture::CompressionMethod::Default, false, true,
						[=, &datas](unsigned char* data, size_t size)
						{
							datas[i].first = new unsigned char[size];
							datas[i].second = size;
							memcpy(datas[i].first, data, size);

						}
					);
					if (_textures[i] == nullptr || !_textures[i]->isLoaded())
					{
						_textures[i] = Texture::getNullTexture();
						datas[i].first = new unsigned char[_textures[i]->getSize()];
						datas[i].second = _textures[i]->getSize();
						memcpy(datas[i].first, _textures[i]->getData(), _textures[i]->getSize());
					}
					else
					{
						if (wIdx == 0)
							wIdx = i;
					}
				}
				else
				{
					int size = _textures[i]->getSize();
					unsigned char* data = _textures[i]->getData();
					datas[i].first = new unsigned char[size];
					datas[i].second = size;
					memcpy(datas[i].first, data, size);
				}
			}
		}

		int size = std::max(_textures[0]->getWidth(), _textures[wIdx]->getHeight());
		bool hasMips = _textures[wIdx]->getNumMipMaps() > 0;
		bgfx::TextureFormat::Enum format = _textures[wIdx]->getFormat();
		size_t texSize = datas[wIdx].second;
		size_t dataSize = texSize * 6;
		
		data = new unsigned char[dataSize];
		memset(data, 0, dataSize);
		for (int i = 0; i < 6; ++i)
		{
			unsigned char* texData = datas[i].first;
			if (datas[i].second != texSize)
			{
				texData = new unsigned char[texSize];
				memset(texData, 0, texSize);
				int _sz = datas[i].second;
				if (_sz > texSize)
					_sz = texSize;
				std::copy(datas[i].first, datas[i].first + _sz, texData);
			}

			std::copy(texData, texData + texSize, data + (texSize * i));

			if (datas[i].second != texSize)
				delete[] texData;
		}

		const bgfx::Memory* mem = bgfx::makeRef(reinterpret_cast<const void*>(data), dataSize, releaseDataCallback);
		textureHandle = bgfx::createTextureCube(size, hasMips, 1, format, BGFX_SAMPLER_UVW_CLAMP, mem);

		for (int i = 0; i < 6; ++i)
		{
			delete[] datas[i].first;
			datas[i].first = nullptr;
			datas[i].second = 0;
		}
	}

	Cubemap* Cubemap::create(std::string location, std::string name)
	{
		Asset* cachedAsset = getLoadedInstance(location, name);
		//assert(cachedAsset == nullptr && "Asset with these location and name already exists");

		Cubemap* cubemap = nullptr;

		if (cachedAsset != nullptr)
		{
			cubemap = (Cubemap*)cachedAsset;
		}
		else
		{
			cubemap = new Cubemap();
			cubemap->setLocation(location);
			cubemap->setName(name);
		}

		cubemap->updateCubemap();
		cubemap->load();

		return cubemap;
	}

	Cubemap* Cubemap::load(std::string location, std::string name)
	{
		if (location.empty())
			return nullptr;

		std::string fullPath = location + name;

		Asset* cachedAsset = getLoadedInstance(location, name);

		if (cachedAsset != nullptr && cachedAsset->isLoaded())
		{
			return (Cubemap*)cachedAsset;
		}
		else
		{
			if (IO::isDir(location))
			{
				if (!IO::FileExists(fullPath))
				{
					Debug::log("[" + fullPath + "] Error loading cubemap: file does not exists", Debug::DbgColorRed);
					return nullptr;
				}
			}
			else
			{
				zip_t* arch = Engine::getSingleton()->getZipArchive(location);
				if (!ZipHelper::isFileInZip(arch, name))
				{
					Debug::log("[" + fullPath + "] Error loading cubemap: file does not exists", Debug::DbgColorRed);
					return nullptr;
				}
			}

			Cubemap* cubemap = nullptr;
			if (cachedAsset == nullptr)
			{
				cubemap = new Cubemap();
				cubemap->setLocation(location);
				cubemap->setName(name);
			}
			else
				cubemap = (Cubemap*)cachedAsset;

			SCubemap sCubemap;

			if (IO::isDir(location))
			{
				try
				{
					std::ifstream ofs(fullPath, std::ios::binary);
					BinarySerializer s;
					s.deserialize(&ofs, &sCubemap, Cubemap::ASSET_TYPE);
					ofs.close();
				}
				catch (std::exception e)
				{
					Debug::log("[" + name + "] Error loading cubemap: " + e.what(), Debug::DbgColorRed);
				}
			}
			else
			{
				zip_t* arch = Engine::getSingleton()->getZipArchive(location);

				int sz = 0;
				char* buffer = ZipHelper::readFileFromZip(arch, name, sz);

				try
				{
					boost::iostreams::stream<boost::iostreams::array_source> is(buffer, sz);
					BinarySerializer s;
					s.deserialize(&is, &sCubemap, Cubemap::ASSET_TYPE);
					is.close();
				}
				catch (std::exception e)
				{
					Debug::log("[" + name + "] Error loading cubemap: " + e.what(), Debug::DbgColorRed);
				}

				delete[] buffer;
			}

			if (sCubemap.textures.size() == 6)
			{
				for (int i = 0; i < 6; ++i)
				{
					std::string assetsDir = Engine::getSingleton()->getAssetsPath();
					std::string texPath = assetsDir + sCubemap.textures[i];
					cubemap->textures[i] = Texture::load(assetsDir, sCubemap.textures[i], true, Texture::CompressionMethod::Default);
				}
			}

			cubemap->updateCubemap();
			cubemap->load();
			
			return cubemap;
		}
	}

	void Cubemap::save()
	{
		SCubemap sCubemap;
		sCubemap.textures.resize(6);

		for (int i = 0; i < 6; ++i)
		{
			if (textures[i] != nullptr)
				sCubemap.textures[i] = textures[i]->getName();
		}

		std::string path = getOrigin();
		std::ofstream ofs(path, std::ios::binary);
		BinarySerializer s;
		s.serialize(&ofs, &sCubemap, Cubemap::ASSET_TYPE);
		ofs.close();
	}

	void Cubemap::setTexture(int index, Texture* texture, bool update)
	{
		assert(index < 6 && "Out of bounds");

		textures[index] = texture;

		if (update)
			updateCubemap();
	}

	void Cubemap::createNullCubemap()
	{
		if (nullCubemap != nullptr)
			return;

		Asset* cachedCubemap = Asset::getLoadedInstance("system/", "NullCubemap");
		if (cachedCubemap != nullptr)
		{
			if (cachedCubemap->isLoaded())
			{
				nullCubemap = (Cubemap*)cachedCubemap;
				return;
			}
			else
				nullCubemap = (Cubemap*)cachedCubemap;
		}
		else
		{
			nullCubemap = new Cubemap();
			nullCubemap->setName("NullCubemap");
			nullCubemap->setLocation("system/");
			nullCubemap->setPersistent(true);

			nullCubemap->updateCubemap();
			nullCubemap->load();
		}
	}

	void Cubemap::destroyNullCubemap()
	{
		if (nullCubemap != nullptr)
		{
			if (Asset::getLoadedInstance("system/", "NullCubemap") != nullptr)
			{
				delete nullCubemap;
			}

			nullCubemap = nullptr;
		}
	}
}