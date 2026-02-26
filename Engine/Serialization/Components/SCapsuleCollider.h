#pragma once

#include "SComponent.h"
#include <string>

namespace GX
{
	class SCapsuleCollider : public SComponent
	{
	public:
		SCapsuleCollider() {}
		~SCapsuleCollider() {}

		virtual void serialize(Serializer* s)
		{
			SComponent::serialize(s);
			data(radius);
			data(height);
			data(offset);
			data(rotation);
			data(isTrigger);
		}

	public:
		float radius = 0.5;
		float height = 2.0;
		SVector3 offset = SVector3(0, 0, 0);
		SQuaternion rotation = SQuaternion(0, 0, 0, 1);
		bool isTrigger = false;
	};
}