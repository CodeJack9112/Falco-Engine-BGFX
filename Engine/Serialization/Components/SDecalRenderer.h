#pragma once

#include <string>

#include "SComponent.h"

namespace GX
{
	class SDecalRenderer : public SComponent
	{
	public:
		SDecalRenderer() {}
		~SDecalRenderer() {}

		virtual int getVersion() { return 0; }

		virtual void serialize(Serializer* s)
		{
			SComponent::serialize(s);
			data(material);
		}

	public:
		std::string material = "";
	};
}