#pragma once

#include <string>
#include <vector>

#include "Asset.h"

namespace GX
{
	class AudioClip : public Asset
	{
	public:
		AudioClip();
		virtual ~AudioClip();

		static std::string ASSET_TYPE;

		virtual void load();
		virtual void unload();
		static void unloadAll();
		virtual void reload();
		virtual std::string getAssetType() { return ASSET_TYPE; }

		static AudioClip* load(std::string location, std::string name, bool warn = true);
	};
}