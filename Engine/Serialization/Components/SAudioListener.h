#pragma once

#include "SComponent.h"
#include <string>

namespace GX
{
	class SAudioListener : public SComponent
	{
	public:
		SAudioListener() {}
		~SAudioListener() {}

		virtual void serialize(Serializer* s)
		{
			SComponent::serialize(s);
			data(volume);
		}

	public:
		float volume = 1.0f;
	};
}