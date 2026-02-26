#pragma once

#include "SComponent.h"
#include <string>

namespace GX
{
	class SSphereCollider : public SComponent
	{
	public:
		SSphereCollider() {}
		~SSphereCollider() {}

		virtual void serialize(Serializer* s)
		{
			SComponent::serialize(s);
			data(radius);
			data(offset);
			data(isTrigger);
		}

	public:
		float radius = 1;
		SVector3 offset = SVector3(0, 0, 0);
		bool isTrigger = false;
	};
}