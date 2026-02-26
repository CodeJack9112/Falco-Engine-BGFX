#include "FixedJoint.h"

#include "Transform.h"
#include "RigidBody.h"
#include "../Math/Mathf.h"
#include "../Core/PhysicsManager.h"
#include "../Core/APIManager.h"
#include "../Core/GameObject.h"
#include "../Core/Engine.h"

namespace GX
{
	std::string FixedJoint::COMPONENT_TYPE = "FixedJoint";

	FixedJoint::FixedJoint() : Component(APIManager::getSingleton()->fixedjoint_class)
	{
	}

	FixedJoint::~FixedJoint()
	{
		removeConstraint();
	}

	std::string FixedJoint::getComponentType()
	{
		return COMPONENT_TYPE;
	}

	Component* FixedJoint::onClone()
	{
		FixedJoint* newComponent = new FixedJoint();
		newComponent->enabled = enabled;
		newComponent->connectedObjectGuid = connectedObjectGuid;
		newComponent->anchor = anchor;
		newComponent->connectedAnchor = connectedAnchor;
		newComponent->linkedBodiesCollision = linkedBodiesCollision;

		return newComponent;
	}

	void FixedJoint::onRebindObject(std::string oldObj, std::string newObj)
	{
		if (oldObj == getConnectedObjectGuid())
			setConnectedObjectGuid(newObj);
	}

	void FixedJoint::onRefresh()
	{
		if (enabled)
		{
			setEnabled(false);
			setEnabled(true);
		}
	}

	void FixedJoint::autoConfigureAnchors()
	{
		GameObject* connectedNode = Engine::getSingleton()->getGameObject(connectedObjectGuid);
		if (connectedNode != nullptr)
		{
			Transform* t = getGameObject()->getTransform();
			glm::vec3 a1 = t->getTransformMatrix() * glm::vec4(anchor, 1.0f);
			connectedAnchor = Mathf::inverseTransformPoint(connectedNode->getTransform(), a1);
		}
	}

	void FixedJoint::onSceneLoaded()
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

							glm::vec3 scl1 = t->getScale();
							glm::vec3 scl2 = connectedNode->getTransform()->getScale();

							glm::vec3 rAnchor = anchor * scl1;
							glm::vec3 rConnectedAnchor = connectedAnchor * scl2;

							localA.setOrigin(btVector3(rAnchor.x, rAnchor.y, rAnchor.z));
							localB.setOrigin(btVector3(rConnectedAnchor.x, rConnectedAnchor.y, rConnectedAnchor.z));

							localA = body->getCenterOfMass().inverse() * localA;
							localB = connectedBody->getCenterOfMass().inverse() * localB;

							if (fixedConstraint != nullptr)
								PhysicsManager::getSingleton()->getWorld()->removeConstraint(fixedConstraint);

							fixedConstraint = new btFixedConstraint(*body1, *body2, localA, localB);

							PhysicsManager::getSingleton()->getWorld()->addConstraint(fixedConstraint, !linkedBodiesCollision);
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

	void FixedJoint::onStateChanged()
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

	void FixedJoint::setConnectedObjectGuid(std::string name)
	{
		connectedObjectGuid = name;
		onRefresh();
	}

	void FixedJoint::removeConstraint()
	{
		if (fixedConstraint != nullptr)
		{
			PhysicsManager::getSingleton()->getWorld()->removeConstraint(fixedConstraint);
			fixedConstraint = nullptr;
		}
	}
}