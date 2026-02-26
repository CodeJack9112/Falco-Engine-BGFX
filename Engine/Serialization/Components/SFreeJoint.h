#pragma once

#include "SComponent.h"
#include <string>

namespace GX
{
	class SFreeJoint : public SComponent
	{
	public:
		SFreeJoint() {}
		~SFreeJoint() {}

		virtual void serialize(Serializer* s)
		{
			SComponent::serialize(s);
			data(connectedObjectGuid);
			data(anchor);
			data(connectedAnchor);
			data(limitMin);
			data(limitMax);
			data(linkedBodiesCollision);
		}

	public:
		std::string connectedObjectGuid = "[None]";
		SVector3 anchor = SVector3(0, 0, 0);
		SVector3 connectedAnchor = SVector3(0, 0, 0);
		SVector3 limitMin = SVector3(-3.14f, -3.14f * 0.5f, -3.14f);
		SVector3 limitMax = SVector3(3.14f, 3.14f * 0.5f, 3.14f);
		SVector3 swingAxis = SVector3(0, 0, 0);
		bool linkedBodiesCollision = true;
	};
}