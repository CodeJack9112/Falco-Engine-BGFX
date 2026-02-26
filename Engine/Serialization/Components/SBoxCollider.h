#pragma once

#include "SComponent.h"
#include <string>

namespace GX
{
	class SBoxCollider : public SComponent
	{
	public:
		SBoxCollider() {}
		~SBoxCollider() {}

		virtual void serialize(Serializer* s)
		{
			SComponent::serialize(s);
			data(boxSize);
			data(offset);
			data(rotation);
			data(isTrigger);
		}

	public:
		SVector3 boxSize = SVector3(1, 1, 1);
		SVector3 offset = SVector3(0, 0, 0);
		SQuaternion rotation = SQuaternion(0, 0, 0, 1);
		bool isTrigger = false;
	};
}