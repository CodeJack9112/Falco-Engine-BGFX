#pragma once

#include "SComponent.h"
#include <string>

namespace GX
{
	class SFixedJoint : public SComponent
	{
	public:
		SFixedJoint() {}
		~SFixedJoint() {}

		virtual void serialize(Serializer* s)
		{
			SComponent::serialize(s);
			data(connectedObjectGuid);
			data(anchor);
			data(connectedAnchor);
			data(linkedBodiesCollision);
		}

	public:
		std::string connectedObjectGuid = "[None]";
		SVector3 anchor = SVector3(0, 0, 0);
		SVector3 connectedAnchor = SVector3(0, 0, 0);
		bool linkedBodiesCollision = true;
	};
}