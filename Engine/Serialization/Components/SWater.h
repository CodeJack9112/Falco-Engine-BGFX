#pragma once

#include <string>

#include "SComponent.h"
#include "../Assets/SMaterial.h"

namespace GX
{
	class SWater : public SComponent
	{
	public:
		SWater() {}
		~SWater() {}

		virtual void serialize(Serializer* s)
		{
			SComponent::serialize(s);
			data(size);
			data(reflections);
			data(material);
			data(reflectSkybox);
			data(reflectObjects);
			data(reflectionsDistance);
			data(reflectionsQuality);
		}

	public:
		int size = 100;
		bool reflections = true;
		bool reflectSkybox = true;
		bool reflectObjects = true;
		float reflectionsDistance = 100;
		int reflectionsQuality = 1;
		SMaterial material;
	};
}