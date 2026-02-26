#pragma once

#include <string>
#include <vector>

#include "Asset.h"

namespace GX
{
	class VideoClip : public Asset
	{
	public:
		VideoClip();
		virtual ~VideoClip();

		static std::string ASSET_TYPE;

		virtual void load();
		virtual void unload();
		static void unloadAll();
		virtual void reload();
		virtual std::string getAssetType() { return ASSET_TYPE; }

		static VideoClip* load(std::string location, std::string name, bool warn = true);

		void extractAudio();
	};
}