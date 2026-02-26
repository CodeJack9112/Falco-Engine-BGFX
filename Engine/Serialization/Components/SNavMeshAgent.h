#pragma once

#include "SComponent.h"
#include <string>

namespace GX
{
	class SNavMeshAgent : public SComponent
	{
	public:
		SNavMeshAgent() {}
		~SNavMeshAgent() {}

		virtual void serialize(Serializer* s)
		{
			SComponent::serialize(s);
			data(radius);
			data(height);
			data(speed);
			data(acceleration);
			data(rotationSpeed);
		}

	public:
		float radius = 0.6f;
		float height = 2.0f;
		float speed = 3.5f;
		float acceleration = 8.0f;
		float rotationSpeed = 1.0f;
	};
}