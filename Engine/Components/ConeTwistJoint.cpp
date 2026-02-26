#include "ConeTwistJoint.h"

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
	std::string ConeTwistJoint::COMPONENT_TYPE = "ConeTwistJoint";

	ConeTwistJoint::ConeTwistJoint() : Component(APIManager::getSingleton()->conetwistjoint_class)
	{
	}

	ConeTwistJoint::~ConeTwistJoint()
	{
		removeConstraint();
	}

	std::string ConeTwistJoint::getComponentType()
	{
		return COMPONENT_TYPE;
	}

	Component* ConeTwistJoint::onClone()
	{
		ConeTwistJoint* newComponent = new ConeTwistJoint();
		newComponent->enabled = enabled;
		newComponent->connectedObjectGuid = connectedObjectGuid;
		newComponent->anchor = anchor;
		newComponent->connectedAnchor = connectedAnchor;
		newComponent->linkedBodiesCollision = linkedBodiesCollision;
		newComponent->limits = limits;

		return newComponent;
	}

	void ConeTwistJoint::onRebindObject(std::string oldObj, std::string newObj)
	{
		if (oldObj == getConnectedObjectGuid())
			setConnectedObjectGuid(newObj);
	}

	void ConeTwistJoint::onRefresh()
	{
		if (enabled)
		{
			setEnabled(false);
			setEnabled(true);
		}
	}

	void ConeTwistJoint::autoConfigureAnchors()
	{
		GameObject* connectedNode = Engine::getSingleton()->getGameObject(connectedObjectGuid);
		if (connectedNode != nullptr)
		{
			Transform* t = getGameObject()->getTransform();
			glm::vec3 a1 = t->getTransformMatrix() * glm::vec4(anchor, 1.0f);
			connectedAnchor = Mathf::inverseTransformPoint(connectedNode->getTransform(), a1);
		}
	}

	void ConeTwistJoint::onSceneLoaded()
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

							glm::highp_quat r1 = glm::inverse(t->getRotation());
							glm::highp_quat r2 = glm::inverse(pt->getRotation());

							localA = body->getCenterOfMass().inverse() * localA;
							localB = connectedBody->getCenterOfMass().inverse() * localB;

							localA.getBasis().setRotation(btQuaternion(r1.x, r1.y, r1.z, r1.w));
							localB.getBasis().setRotation(btQuaternion(r2.x, r2.y, r2.z, r2.w));

							if (coneTwistConstraint != nullptr)
								PhysicsManager::getSingleton()->getWorld()->removeConstraint(coneTwistConstraint);

							coneTwistConstraint = new btConeTwistConstraint(*body1, *body2, localA, localB);
							coneTwistConstraint->setLimit(limits.x, limits.y, limits.z);

							PhysicsManager::getSingleton()->getWorld()->addConstraint(coneTwistConstraint, !linkedBodiesCollision);
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

	void ConeTwistJoint::onStateChanged()
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

	void ConeTwistJoint::setConnectedObjectGuid(std::string name)
	{
		connectedObjectGuid = name;
		onRefresh();
	}

	void ConeTwistJoint::removeConstraint()
	{
		if (coneTwistConstraint != nullptr)
		{
			PhysicsManager::getSingleton()->getWorld()->removeConstraint(coneTwistConstraint);
			coneTwistConstraint = nullptr;
		}
	}
}