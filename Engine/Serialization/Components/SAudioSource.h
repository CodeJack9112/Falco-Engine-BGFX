#pragma once

#include "SComponent.h"
#include <string>

namespace GX
{
	class SAudioSource : public SComponent
	{
	public:
		SAudioSource() {}
		~SAudioSource() {}

		virtual void serialize(Serializer* s)
		{
			SComponent::serialize(s);
			data(fileName);
			data(playOnStart);
			data(loop);
			data(volume);
			data(minDistance);
			data(maxDistance);
			data(is2D);
			data(pitch);
		}

	public:
		std::string fileName;
		bool playOnStart = false;
		bool loop = false;
		bool is2D = false;
		float volume = 1.0f;
		float minDistance = 10.0f;
		float maxDistance = 1000.0f;
		float pitch = 1.0f;
	};
}