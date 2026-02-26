#include "HingeJoint.h"

#include "Transform.h"
#include "RigidBody.h"
#include "../Math/Mathf.h"
#include "../Core/PhysicsManager.h"
#include "../Core/APIManager.h"
#include "../Core/GameObject.h"
#include "../Core/Engine.h"

#include "../Bullet/include/btBulletCollisionCommon.h"
#include "../Bullet/include/btBulletDynamicsCommon.h"

namespace GX
{
	std::string HingeJoint::COMPONENT_TYPE = "HingeJoint";

	HingeJoint::HingeJoint() : Component(APIManager::getSingleton()->hingejoint_class)
	{
	}

	HingeJoint::~HingeJoint()
	{
		removeConstraint();
	}

	std::string HingeJoint::getComponentType()
	{
		return COMPONENT_TYPE;
	}

	Component* HingeJoint::onClone()
	{
		HingeJoint* newComponent = new HingeJoint();
		newComponent->enabled = enabled;
		newComponent->connectedObjectGuid = connectedObjectGuid;
		newComponent->anchor = anchor;
		newComponent->connectedAnchor = connectedAnchor;
		newComponent->linkedBodiesCollision = linkedBodiesCollision;
		newComponent->limitMin = limitMin;
		newComponent->limitMax = limitMax;
		newComponent->axis = axis;

		return newComponent;
	}

	void HingeJoint::onRebindObject(std::string oldObj, std::string newObj)
	{
		if (oldObj == getConnectedObjectGuid())
			setConnectedObjectGuid(newObj);
	}

	void HingeJoint::onRefresh()
	{
		if (enabled)
		{
			setEnabled(false);
			setEnabled(true);
		}
	}

	void HingeJoint::autoConfigureAnchors()
	{
		GameObject* connectedNode = Engine::getSingleton()->getGameObject(connectedObjectGuid);
		if (connectedNode != nullptr)
		{
			Transform* t = getGameObject()->getTransform();
			glm::vec3 a1 = t->getTransformMatrix() * glm::vec4(anchor, 1.0f);
			connectedAnchor = Mathf::inverseTransformPoint(connectedNode->getTransform(), a1);
		}
	}

	int calculateDirection(glm::vec3 point)
	{
		// Calculate longest axis
		int direction = 0;
		if (abs(point[1]) > abs(point[0]))
			direction = 1;
		if (abs(point[2]) > abs(point[direction]))
			direction = 2;

		return direction;
	}

	void HingeJoint::onSceneLoaded()
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

							if (hingeConstraint != nullptr)
								PhysicsManager::getSingleton()->getWorld()->removeConstraint(hingeConstraint);

							btVector3 ax = btVector3(axis.x, axis.y, axis.z);

							hingeConstraint = new btHingeConstraint(*body1, *body2, localA, localB, true);
							hingeConstraint->setAxis(ax);
							hingeConstraint->setLimit(limitMin, limitMax);

							PhysicsManager::getSingleton()->getWorld()->addConstraint(hingeConstraint, !linkedBodiesCollision);
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

	void HingeJoint::onStateChanged()
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

	void HingeJoint::setConnectedObjectGuid(std::string name)
	{
		connectedObjectGuid = name;
		onRefresh();
	}

	void HingeJoint::removeConstraint()
	{
		if (hingeConstraint != nullptr)
		{
			PhysicsManager::getSingleton()->getWorld()->removeConstraint(hingeConstraint);
			hingeConstraint = nullptr;
		}
	}
}