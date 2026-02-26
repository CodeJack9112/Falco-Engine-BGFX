#pragma once

#include "SComponent.h"
#include <string>

namespace GX
{
	class SNavMeshObstacle : public SComponent
	{
	public:
		SNavMeshObstacle() {}
		~SNavMeshObstacle() {}

		virtual void serialize(Serializer* s)
		{
			SComponent::serialize(s);
			data(size);
			data(offset);
		}

	public:
		SVector3 size = SVector3(1, 1, 1);
		SVector3 offset = SVector3(0, 0, 0);
	};
}