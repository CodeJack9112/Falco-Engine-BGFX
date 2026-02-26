#pragma once

#pragma once

#include <string>
#include <vector>

#include "Asset.h"

#include "../Serialization/Scene/SScene.h"

namespace GX
{
	class Prefab : public Asset
	{
	private:
		SScene scene;

	public:
		Prefab();
		virtual ~Prefab();

		static std::string ASSET_TYPE;

		virtual void load();
		virtual void unload();
		static void unloadAll();
		virtual void reload();
		virtual std::string getAssetType() { return ASSET_TYPE; }

		static Prefab* load(std::string location, std::string name, bool warn = true);

		SScene& getScene() { return scene; }
	};
}