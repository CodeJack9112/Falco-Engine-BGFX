#pragma once

#include "SComponent.h"
#include <string>

namespace GX
{
	class SHingeJoint : public SComponent
	{
	public:
		SHingeJoint() {}
		~SHingeJoint() {}

		virtual void serialize(Serializer* s)
		{
			SComponent::serialize(s);
			data(connectedObjectGuid);
			data(anchor);
			data(connectedAnchor);
			data(axis);
			data(limitMin);
			data(limitMax);
			data(linkedBodiesCollision);
		}

	public:
		std::string connectedObjectGuid = "[None]";
		SVector3 anchor = SVector3(0, 0, 0);
		SVector3 connectedAnchor = SVector3(0, 0, 0);
		SVector3 axis = SVector3(1, 0, 0);
		float limitMin = -3.14f;
		float limitMax = 3.14f;
		bool linkedBodiesCollision = true;
	};
}