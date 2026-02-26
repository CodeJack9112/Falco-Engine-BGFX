#pragma once

#include <string>

#include "SComponent.h"

namespace GX
{
	class SCanvas : public SComponent
	{
	public:
		SCanvas() = default;
		~SCanvas() {}

		virtual void serialize(Serializer* s)
		{
			SComponent::serialize(s);
			data(refScreenWidth);
			data(refScreenHeight);
			data(screenMatchSide);
			data(mode);
		}

	public:
		int refScreenWidth = 1920;
		int refScreenHeight = 1080;
		float screenMatchSide = 1.0f;
		int mode = 0;
	};
}