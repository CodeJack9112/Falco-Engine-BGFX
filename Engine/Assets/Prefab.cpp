#include "Prefab.h"

#include <boost/iostreams/stream.hpp>

#include "../Core/Engine.h"
#include "../Core/APIManager.h"
#include "../Core/Debug.h"
#include "../Classes/IO.h"
#include "../Classes/ZipHelper.h"

namespace GX
{
	std::string Prefab::ASSET_TYPE = "Prefab";

	Prefab::Prefab() : Asset(APIManager::getSingleton()->prefab_class)
	{
	}

	Prefab::~Prefab()
	{
	}

	void Prefab::unload()
	{
		if (isLoaded())
		{
			Asset::unload();
		}
	}

	void Prefab::unloadAll()
	{
		std::vector<Prefab*> deleteAssets;

		for (auto it = loadedInstances.begin(); it != loadedInstances.end(); ++it)
		{
			if (it->second->getAssetType() == Prefab::ASSET_TYPE)
			{
				if (!it->second->getPersistent())
					deleteAssets.push_back((Prefab*)it->second);
			}
		}

		for (auto it = deleteAssets.begin(); it != deleteAssets.end(); ++it)
			delete (*it);

		deleteAssets.clear();
	}

	void Prefab::load()
	{
		if (!isLoaded())
		{
			Asset::load();
		}
	}

	void Prefab::reload()
	{
		if (getOrigin().empty())
			return;

		if (isLoaded())
			unload();

		load(location, name);
	}

	Prefab* Prefab::load(std::string location, std::string name, bool warn)
	{
		std::string fullPath = location + name;

		Asset* cachedAsset = getLoadedInstance(location, name);

		if (cachedAsset != nullptr && cachedAsset->isLoaded())
		{
			return (Prefab*)cachedAsset;
		}
		else
		{
			Prefab* prefab = nullptr;
			if (cachedAsset == nullptr)
			{
				prefab = new Prefab();
				prefab->setLocation(location);
				prefab->setName(name);
			}
			else
				prefab = (Prefab*)cachedAsset;

			if (IO::isDir(location))
			{
				if (!IO::FileExists(fullPath))
				{
					if (warn)
						Debug::logWarning("[" + fullPath + "] Error loading prefab: file does not exists");
					return nullptr;
				}
			}
			else
			{
				zip_t* arch = Engine::getSingleton()->getZipArchive(location);
				if (!ZipHelper::isFileInZip(arch, name))
				{
					if (warn)
						Debug::logWarning("[" + fullPath + "] Error loading prefab: file does not exists");
					return nullptr;
				}
			}

			if (IO::isDir(location))
			{
				try
				{
					std::ifstream ofs(fullPath, std::ios::binary);
					BinarySerializer s;
					s.deserialize(&ofs, &prefab->scene, Prefab::ASSET_TYPE);
					ofs.close();
				}
				catch (std::exception e)
				{
					Debug::log("[" + name + "] Error loading prefab: " + e.what());
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
					s.deserialize(&is, &prefab->scene, Prefab::ASSET_TYPE);
					is.close();
				}
				catch (std::exception e)
				{
					Debug::log("[" + name + "] Error loading prefab: " + e.what());
				}

				delete[] buffer;
			}

			prefab->load();

			return prefab;
		}
	}
}