#include "AudioClip.h"

#include "../Core/Engine.h"
#include "../Core/APIManager.h"
#include "../Core/Debug.h"
#include "../Classes/IO.h"
#include "../Classes/ZipHelper.h"

namespace GX
{
	std::string AudioClip::ASSET_TYPE = "AudioClip";

	AudioClip::AudioClip() : Asset(APIManager::getSingleton()->audioclip_class)
	{

	}

	AudioClip::~AudioClip()
	{
	}

	void AudioClip::unload()
	{
		if (isLoaded())
		{
			Asset::unload();
		}
	}

	void AudioClip::unloadAll()
	{
		std::vector<AudioClip*> deleteAssets;

		for (auto it = loadedInstances.begin(); it != loadedInstances.end(); ++it)
		{
			if (it->second->getAssetType() == AudioClip::ASSET_TYPE)
			{
				if (!it->second->getPersistent())
					deleteAssets.push_back((AudioClip*)it->second);
			}
		}

		for (auto it = deleteAssets.begin(); it != deleteAssets.end(); ++it)
			delete (*it);

		deleteAssets.clear();
	}

	void AudioClip::load()
	{
		if (!isLoaded())
		{
			Asset::load();
		}
	}

	void AudioClip::reload()
	{
		if (getOrigin().empty())
			return;

		if (isLoaded())
			unload();

		load(location, name);
	}

	AudioClip* AudioClip::load(std::string location, std::string name, bool warn)
	{
		std::string fullPath = location + name;

		Asset* cachedAsset = getLoadedInstance(location, name);

		if (cachedAsset != nullptr && cachedAsset->isLoaded())
		{
			return (AudioClip*)cachedAsset;
		}
		else
		{
			AudioClip* clip = nullptr;
			if (cachedAsset == nullptr)
			{
				clip = new AudioClip();
				clip->setLocation(location);
				clip->setName(name);
			}
			else
				clip = (AudioClip*)cachedAsset;

			if (IO::isDir(location))
			{
				if (!IO::FileExists(fullPath))
				{
					if (warn)
						Debug::logWarning("[" + fullPath + "] Error loading audio clip: file does not exists");
					return nullptr;
				}
			}
			else
			{
				zip_t* arch = Engine::getSingleton()->getZipArchive(location);
				if (!ZipHelper::isFileInZip(arch, name))
				{
					if (warn)
						Debug::logWarning("[" + fullPath + "] Error loading audio clip: file does not exists");
					return nullptr;
				}
			}

			clip->load();

			return clip;
		}
	}
}