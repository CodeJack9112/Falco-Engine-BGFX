#pragma once

#include "SComponent.h"
#include <string>

namespace GX
{
	struct SWheelInfo : public Archive
	{
	public:
		SWheelInfo() {}
		~SWheelInfo() {}

		virtual void serialize(Serializer* s)
		{
			Archive::serialize(s);
			data(m_connectedObjectGuid);
			data(m_radius);
			data(m_width);
			data(m_suspensionStiffness);
			data(m_suspensionDamping);
			data(m_suspensionCompression);
			data(m_suspensionRestLength);
			data(m_friction);
			data(m_rollInfluence);
			data(m_direction);
			data(m_axle);
			data(m_connectionPoint);
			data(m_isFrontWheel);
		}

	public:
		std::string m_connectedObjectGuid = "[None]";
		float m_radius = 0.5f;
		float m_width = 0.4f;
		float m_suspensionStiffness = 20.0f;
		float m_suspensionDamping = 2.3f;
		float m_suspensionCompression = 4.4f;
		float m_suspensionRestLength = 0.6f;
		float m_friction = 1000.0f;
		float m_rollInfluence = 0.1f;
		SVector3 m_direction = SVector3(0, -1, 0);
		SVector3 m_axle = SVector3(-1, 0, 0);
		SVector3 m_connectionPoint = SVector3(0, 0, 0);
		bool m_isFrontWheel = false;
	};

	class SVehicle : public SComponent
	{
	public:
		SVehicle() {}
		~SVehicle() {}

		virtual void serialize(Serializer* s)
		{
			SComponent::serialize(s);
			data(vehicleAxis);
			data(invertForward);
			data(wheels);
		}

	public:
		std::vector<SWheelInfo> wheels;

		SVector3 vehicleAxis = SVector3(0, 1, 2);
		bool invertForward = false;
	};
}