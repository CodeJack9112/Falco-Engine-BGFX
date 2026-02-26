#pragma once

#include "SComponent.h"
#include <string>

namespace GX
{
	class SMeshCollider : public SComponent
	{
	public:
		SMeshCollider() {}
		~SMeshCollider() {}

		virtual void serialize(Serializer* s)
		{
			SComponent::serialize(s);
			data(convex);
			data(isTrigger);
		}

	public:
		bool convex = false;
		bool isTrigger = false;
	};
}