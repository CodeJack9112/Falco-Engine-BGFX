#pragma once

#include "SComponent.h"
#include <string>

namespace GX
{
	class SConeTwistJoint : public SComponent
	{
	public:
		SConeTwistJoint() {}
		~SConeTwistJoint() {}

		virtual void serialize(Serializer* s)
		{
			SComponent::serialize(s);
			data(connectedObjectGuid);
			data(anchor);
			data(connectedAnchor);
			data(limits);
			data(linkedBodiesCollision);
		}

	public:
		std::string connectedObjectGuid = "[None]";
		SVector3 anchor = SVector3(0, 0, 0);
		SVector3 connectedAnchor = SVector3(0, 0, 0);
		SVector3 limits = SVector3(0, 0, 0);
		SQuaternion rotation = SQuaternion(0, 0, 0, 1);
		bool linkedBodiesCollision = true;
	};
}