#pragma once

#include <string>

#include "SComponent.h"
#include "../Data/SColor.h"

namespace GX
{
	class SLight : public SComponent
	{
	public:
		SLight() {}
		~SLight() {}

		virtual void serialize(Serializer* s)
		{
			SComponent::serialize(s);
			data(color);
			data(lightType);
			data(radius);
			data(intensity);
			data(innerRadius);
			data(outerRadius);
			data(bias);
			data(castShadows);
			data(lightRenderMode);
		}

	public:
		SColor color = SColor(1.0f, 1.0f, 1.0f, 1.0f);
		int lightType = 0;
		int lightRenderMode = 0;
		float radius = 10.0f;
		float innerRadius = 35.0f;
		float outerRadius = 45.0f;
		float intensity = 1.0f;
		float bias = 0.0011f;
		bool castShadows = true;
	};
}