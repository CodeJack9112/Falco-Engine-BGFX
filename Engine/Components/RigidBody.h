#pragma once

#include "Component.h"
#include <string>

#include "../glm/vec3.hpp"
#include "../glm/gtx/quaternion.hpp"

#include "../Bullet/include/btBulletCollisionCommon.h"
#include "../Bullet/include/btBulletDynamicsCommon.h"

namespace GX
{
	class RigidBody : public Component
	{
	private:
		btDefaultMotionState * motionState = nullptr;
		btRigidBody * body = nullptr;
		btTransform startTransform;
		btTransform centerOfMass;
		glm::highp_quat initialRotation = glm::identity<glm::highp_quat>();
		int collisionFlags = 0;

		//------------------------------------
		btScalar mass = 1.0f;
		glm::vec3 gravity = glm::vec3(0, -9.81f, 0);
		float friction = 0.85f;
		float bounciness = 0.25f;
		float linearDamping = 0.0f;
		float angularDamping = 0.0f;

		bool freezePositionX = false;
		bool freezePositionY = false;
		bool freezePositionZ = false;
		bool freezeRotationX = false;
		bool freezeRotationY = false;
		bool freezeRotationZ = false;
		bool isKinematic = false;

		bool isStatic = false;
		bool useOwnGravity = false;

		bool isTrigger = false;
		//------------------------------------

		bool physicsActive = false;
		bool initialized = false;

		void setupPhysics();
		void processColliders();
		void updateVehicles(btRigidBody* oldBody);

	public:
		RigidBody();
		virtual ~RigidBody();

		void update();
		void reload();

		float getMass();
		void setMass(float mass);

		bool getIsStatic() { return isStatic; }
		void setIsStatic(bool stat);

		bool getUseOwnGravity() { return useOwnGravity; }
		void setUseOwnGravity(bool val) { useOwnGravity = val; }

		glm::vec3 getGravity() { return gravity; }
		void setGravity(glm::vec3 value);

		float getFriction() { return friction; }
		void setFriction(float value);

		float getLinearDamping() { return linearDamping; }
		void setLinearDamping(float value);

		float getAngularDamping() { return angularDamping; }
		void setAngularDamping(float value);

		float getBounciness() { return bounciness; }
		void setBounciness(float value);

		bool getIsTrigger() { return isTrigger; }

		btTransform getCenterOfMass() { return centerOfMass; }

		btRigidBody * getNativeBody() { return body; }

		glm::vec3 getPosition();
		glm::highp_quat getRotation();
		void setPosition(glm::vec3 position);
		void setRotation(glm::highp_quat rotation);
		void addForce(glm::vec3 force, glm::vec3 pos);
		void addTorque(glm::vec3 torque);
		glm::vec3 getLinearVelocity();
		void setLinearVelocity(glm::vec3 velocity);
		glm::vec3 getAngularVelocity();
		void setAngularVelocity(glm::vec3 velocity);

		void setFreezePositionX(bool value);
		void setFreezePositionY(bool value);
		void setFreezePositionZ(bool value);
		void setFreezeRotationX(bool value);
		void setFreezeRotationY(bool value);
		void setFreezeRotationZ(bool value);
		void setIsKinematic(bool value);

		bool getFreezePositionX() { return freezePositionX; }
		bool getFreezePositionY() { return freezePositionY; }
		bool getFreezePositionZ() { return freezePositionZ; }
		bool getFreezeRotationX() { return freezeRotationX; }
		bool getFreezeRotationY() { return freezeRotationY; }
		bool getFreezeRotationZ() { return freezeRotationZ; }
		bool getIsKinematic() { return isKinematic; }
		bool isInitialized() { return initialized; }

		static std::string COMPONENT_TYPE;
		virtual std::string getComponentType();
		virtual Component* onClone();
		virtual void onRefresh();
		virtual void onAttach();
		virtual void onDetach();
		virtual void onSceneLoaded();
		virtual void onStateChanged();

		void updateCollisionMask();
	};
}