#pragma once

#include <vector>

#include "SGameObject.h"
#include "SNavMeshSettings.h"

namespace GX
{
	class SScene : public Archive
	{
	public:
		SScene() {}
		~SScene() {}

		virtual int getVersion() { return 2; }

		virtual void serialize(Serializer* s)
		{
			Archive::serialize(s);
			data(ambientColor);
			data(navMeshSettings);
			data(skyboxMaterial);
			data(lightmapSize);
			data(skyModel);
			data(gameObjects);
			if (version > 0)
			{
				data(giEnabled);
				data(giIntensity);
			}
			if (version > 1)
			{
				data(fogEnabled);
				data(fogIncludeSkybox);
				data(fogStartDistance);
				data(fogEndDistance);
				data(fogDensity);
				data(fogColor);
				data(fogType);
			}
		}

		//Serialize members
		int skyModel = 0;
		SColor ambientColor = SColor(0.7f, 0.7f, 0.7f, 1.0f);
		bool giEnabled = true;
		float giIntensity = 1.0f;
		std::string skyboxMaterial = "";
		int lightmapSize = 256;
		bool fogEnabled = false;
		bool fogIncludeSkybox = false;
		float fogStartDistance = 10.0f;
		float fogEndDistance = 100.0f;
		float fogDensity = 0.25f;
		SColor fogColor = Color::White;
		int fogType = 0;
		SNavMeshSettings navMeshSettings;
		std::vector<SGameObject> gameObjects;
	};
}