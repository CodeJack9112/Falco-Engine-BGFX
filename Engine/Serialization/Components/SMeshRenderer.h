#pragma once

#include <string>

#include "SComponent.h"

namespace GX
{
	class SMeshRenderer : public SComponent
	{
	public:
		SMeshRenderer() {}
		~SMeshRenderer() {}

		virtual int getVersion() { return 2; }

		virtual void serialize(Serializer* s)
		{
			SComponent::serialize(s);
			data(materials);
			data(mesh);
			data(meshSourceFile);
			data(rootObjectGuid);
			data(castShadows);
			data(lightmapSize);
			if (version > 0)
				data(lodMaxDistance);
			if (version > 1)
				data(cullOverMaxDistance);
		}

	public:
		std::vector<std::string> materials;
		std::string mesh = "";
		std::string meshSourceFile = "";
		std::string rootObjectGuid = "";
		bool castShadows = true;
		unsigned char lightmapSize = 0;
		float lodMaxDistance = 50.0f;
		bool cullOverMaxDistance = false;
	};
}