#pragma once

#include "SComponent.h"
#include <string>

namespace GX
{
	class SRigidBody : public SComponent
	{
	public:
		SRigidBody() {}
		~SRigidBody() {}

		virtual void serialize(Serializer* s)
		{
			SComponent::serialize(s);
			data(mass);
			data(freezePositionX);
			data(freezePositionY);
			data(freezePositionZ);
			data(freezeRotationX);
			data(freezeRotationY);
			data(freezeRotationZ);
			data(isKinematic);
			data(isStatic);
			data(useOwnGravity);
			data(gravity);
			data(friction);
			data(bounciness);
			data(linearDamping);
			data(angularDamping);
		}

	public:
		float mass = 0;
		bool freezePositionX = false;
		bool freezePositionY = false;
		bool freezePositionZ = false;

		bool freezeRotationX = false;
		bool freezeRotationY = false;
		bool freezeRotationZ = false;

		bool isKinematic = false;
		bool isStatic = false;

		bool useOwnGravity = false;
		SVector3 gravity = SVector3(0, -9.81f, 0);
		float friction = 0.85f;
		float bounciness = 0.25f;
		float linearDamping = 0.0f;
		float angularDamping = 0.0f;
	};
}