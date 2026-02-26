#pragma once

#include "../Serializers/BinarySerializer.h"

namespace GX
{
	struct SNavMeshSettings : public Archive
	{
	public:

		virtual void serialize(Serializer* s)
		{
			Archive::serialize(s);
			data(walkableSlopeAngle);
			data(walkableHeight);
			data(walkableClimb);
			data(walkableRadius);
			data(maxEdgeLen);
			data(maxSimplificationError);
			data(minRegionArea);
			data(mergeRegionArea);
			data(cellSize);
			data(cellHeight);
		}

		SNavMeshSettings() {}
		~SNavMeshSettings() {}

		float walkableSlopeAngle = 45.0f;
		float walkableHeight = 2.0f;
		float walkableClimb = 3.9f;
		float walkableRadius = 1.6f;
		float maxEdgeLen = 12.0f;
		float maxSimplificationError = 3.0f;
		float minRegionArea = 4.0f;
		float mergeRegionArea = 20.0f;
		float cellSize = 1.00f;
		float cellHeight = 0.20f;
	};
}