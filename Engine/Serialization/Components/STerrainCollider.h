#pragma once

#include "SComponent.h"
#include <string>

namespace GX
{
	class STerrainCollider : public SComponent
	{
	public:
		STerrainCollider() {}
		~STerrainCollider() {}

		virtual void serialize(Serializer* s)
		{
			SComponent::serialize(s);
			data(isTrigger);
		}

	public:
		bool isTrigger = false;
	};
}