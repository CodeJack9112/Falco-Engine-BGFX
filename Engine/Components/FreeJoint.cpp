#include "FreeJoint.h"

#include "Transform.h"
#include "RigidBody.h"
#include "../Math/Mathf.h"
#include "../Core/PhysicsManager.h"
#include "../Core/APIManager.h"
#include "../Core/GameObject.h"
#include "../Core/Engine.h"

#include "../Bullet/include/btBulletCollisionCommon.h"
#include "../Bullet/include/btBulletDynamicsCommon.h"

#include "../glm/gtx/vector_angle.hpp"

#define EPSILON 0.00000001f;

namespace GX
{
	std::string FreeJoint::COMPONENT_TYPE = "FreeJoint";
	
	FreeJoint::FreeJoint() : Component(APIManager::getSingleton()->freejoint_class)
	{
	}

	FreeJoint::~FreeJoint()
	{
		removeConstraint();
	}

	std::string FreeJoint::getComponentType()
	{
		return COMPONENT_TYPE;
	}

	Component* FreeJoint::onClone()
	{
		FreeJoint* newComponent = new FreeJoint();
		newComponent->enabled = enabled;
		newComponent->connectedObjectGuid = connectedObjectGuid;
		newComponent->anchor = anchor;
		newComponent->connectedAnchor = connectedAnchor;
		newComponent->linkedBodiesCollision = linkedBodiesCollision;
		newComponent->limitMin = limitMin;
		newComponent->limitMax = limitMax;

		return newComponent;
	}

	void FreeJoint::onRebindObject(std::string oldObj, std::string newObj)
	{
		if (oldObj == getConnectedObjectGuid())
			setConnectedObjectGuid(newObj);
	}

	void FreeJoint::onRefresh()
	{
		if (enabled)
		{
			setEnabled(false);
			setEnabled(true);
		}
	}

	void FreeJoint::autoConfigureAnchors()
	{
		GameObject* connectedNode = Engine::getSingleton()->getGameObject(connectedObjectGuid);
		if (connectedNode != nullptr)
		{
			Transform* t = getGameObject()->getTransform();
			glm::vec3 a1 = t->getTransformMatrix() * glm::vec4(anchor, 1.0f);
			connectedAnchor = Mathf::inverseTransformPoint(connectedNode->getTransform(), a1);
		}
	}

	void FreeJoint::onSceneLoaded()
	{
		if (!getEnabled() || !getGameObject()->getActive())
			return;

		if (Engine::getSingleton()->getIsRuntimeMode())
		{
			RigidBody* body = (RigidBody*)getGameObject()->getComponent(RigidBody::COMPONENT_TYPE);

			if (body != nullptr)
			{
				GameObject* connectedNode = Engine::getSingleton()->getGameObject(connectedObjectGuid);
				if (connectedNode != nullptr)
				{
					RigidBody* connectedBody = (RigidBody*)connectedNode->getComponent(RigidBody::COMPONENT_TYPE);
					FreeJoint* connectedJoint = (FreeJoint*)connectedNode->getComponent(FreeJoint::COMPONENT_TYPE);

					if (connectedBody != nullptr)
					{
						btRigidBody* body1 = body->getNativeBody();
						btRigidBody* body2 = connectedBody->getNativeBody();

						if (body1 != nullptr && body2 != nullptr)
						{
							btTransform localA, localB;

							localA.setIdentity();
							localB.setIdentity();

							Transform* t = getGameObject()->getTransform();
							Transform* pt = connectedNode->getTransform();

							glm::vec3 scl1 = t->getScale();
							glm::vec3 scl2 = connectedNode->getTransform()->getScale();

							glm::vec3 rAnchor = anchor * scl1;
							glm::vec3 rConnectedAnchor = connectedAnchor * scl2;

							localA.setOrigin(btVector3(rAnchor.x, rAnchor.y, rAnchor.z));
							localB.setOrigin(btVector3(rConnectedAnchor.x, rConnectedAnchor.y, rConnectedAnchor.z));

							localA = body->getCenterOfMass().inverse() * localA;
							localB = connectedBody->getCenterOfMass().inverse() * localB;
							
							//glm::highp_quat r1 = glm::inverse(t->getRotation());
							//glm::highp_quat r2 = glm::inverse(pt->getRotation());

							//localA.setRotation(btQuaternion(r1.x, r1.y, r1.z, r1.w));
							//localB.setRotation(btQuaternion(r2.x, r2.y, r2.z, r2.w));

							if (freeConstraint != nullptr)
								PhysicsManager::getSingleton()->getWorld()->removeConstraint(freeConstraint);

							freeConstraint = new btGeneric6DofConstraint(*body1, *body2, localA, localB, true);

							glm::vec3 limMin = limitMin;
							glm::vec3 limMax = limitMax;

							freeConstraint->setAngularLowerLimit(btVector3(limMin.x, limMin.y, limMin.z));
							freeConstraint->setAngularUpperLimit(btVector3(limMax.x, limMax.y, limMax.z));
							freeConstraint->setOverrideNumSolverIterations(24);

							PhysicsManager::getSingleton()->getWorld()->addConstraint(freeConstraint, !linkedBodiesCollision);
						}
						else
						{
							removeConstraint();
						}
					}
					else
					{
						removeConstraint();
					}
				}
				else
				{
					removeConstraint();
				}
			}
		}
	}

	void FreeJoint::onStateChanged()
	{
		if (getGameObject() == nullptr)
			return;

		bool active = getEnabled() && getGameObject()->getActive();

		if (active)
		{
			RigidBody* body = (RigidBody*)getGameObject()->getComponent(RigidBody::COMPONENT_TYPE);

			if (body != nullptr)
			{
				GameObject* connectedNode = Engine::getSingleton()->getGameObject(connectedObjectGuid);
				if (connectedNode != nullptr)
				{
					RigidBody* connectedBody = (RigidBody*)connectedNode->getComponent(RigidBody::COMPONENT_TYPE);

					if (connectedBody != nullptr)
					{
						if (body->isInitialized() && connectedBody->isInitialized())
						{
							onSceneLoaded();
						}
					}
				}
			}
		}
		else
		{
			removeConstraint();
		}
	}

	void FreeJoint::setConnectedObjectGuid(std::string name)
	{
		connectedObjectGuid = name;
		onRefresh();
	}

	void FreeJoint::setLimitMin(glm::vec3 value)
	{
		limitMin = value;

		if (limitMin.x == 0.0f) limitMin.x = -EPSILON;
		if (limitMin.y == 0.0f) limitMin.y = -EPSILON;
		if (limitMin.z == 0.0f) limitMin.z = -EPSILON;
	}

	void FreeJoint::setLimitMax(glm::vec3 value)
	{
		limitMax = value;

		if (limitMax.x == 0.0f) limitMax.x = EPSILON;
		if (limitMax.y == 0.0f) limitMax.y = EPSILON;
		if (limitMax.z == 0.0f) limitMax.z = EPSILON;
	}

	void FreeJoint::removeConstraint()
	{
		if (freeConstraint != nullptr)
		{
			PhysicsManager::getSingleton()->getWorld()->removeConstraint(freeConstraint);
			freeConstraint = nullptr;
		}
	}
}