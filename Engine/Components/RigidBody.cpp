#include "RigidBody.h"

#include <iostream>

#include "Collider.h"
#include "Transform.h"
#include "Classes/GUIDGenerator.h"
#include "../Core/GameObject.h"
#include "../Core/PhysicsManager.h"
#include "../Core/APIManager.h"
#include "../Core/Engine.h"
#include "../Components/MeshRenderer.h"
#include "../Components/Vehicle.h"

namespace GX
{
	std::string RigidBody::COMPONENT_TYPE = "Rigidbody";

	btVector3 toBulletVec3(glm::vec3 value) { return btVector3(value.x, value.y, value.z); }
	glm::vec3 fromBulletVec3(btVector3 value) { return glm::vec3(value.getX(), value.getY(), value.getZ()); }
	btQuaternion toBulletQuat(glm::highp_quat value) { return btQuaternion(value.x, value.y, value.z, value.w); }
	glm::highp_quat fromBulletQuat(btQuaternion value) { return glm::highp_quat(value.getW(), value.getX(), value.getY(), value.getZ()); }

	RigidBody::RigidBody() : Component(APIManager::getSingleton()->rigidbody_class)
	{
		
	}

	RigidBody::~RigidBody()
	{
		physicsActive = false;

		if (body != nullptr)
		{
			if (body->getCollisionShape() != nullptr)
				delete body->getCollisionShape();

			PhysicsManager::getSingleton()->getWorld()->removeRigidBody(body);
			delete body;
		}

		if (motionState != nullptr)
			delete motionState;
	}

	std::string RigidBody::getComponentType()
	{
		return COMPONENT_TYPE;
	}

	Component* RigidBody::onClone()
	{
		RigidBody* newComponent = new RigidBody();
		newComponent->enabled = enabled;
		newComponent->mass = mass;
		newComponent->gravity = gravity;
		newComponent->friction = friction;
		newComponent->bounciness = bounciness;
		newComponent->linearDamping = linearDamping;
		newComponent->angularDamping = angularDamping;
		newComponent->freezePositionX = freezePositionX;
		newComponent->freezePositionY = freezePositionY;
		newComponent->freezePositionZ = freezePositionZ;
		newComponent->freezeRotationX = freezeRotationX;
		newComponent->freezeRotationY = freezeRotationY;
		newComponent->freezeRotationZ = freezeRotationZ;
		newComponent->isKinematic = isKinematic;
		newComponent->isStatic = isStatic;
		newComponent->useOwnGravity = useOwnGravity;
		newComponent->isTrigger = isTrigger;

		return newComponent;
	}

	void RigidBody::onRefresh()
	{
		reload();
	}

	void RigidBody::setupPhysics()
	{
		if (!initialized)
			return;

		if (!getEnabled() || !getGameObject()->getActive())
			return;

		processColliders();

		collisionFlags = body->getCollisionFlags();
		setIsKinematic(getIsKinematic());

		physicsActive = true;

		if (useOwnGravity)
			body->setGravity(btVector3(gravity.x, gravity.y, gravity.z));

		updateCollisionMask();

		PhysicsManager::getSingleton()->addBody(this);
	}

	void RigidBody::processColliders()
	{
		if (!getEnabled() || !getGameObject()->getActive())
			return;

		btRigidBody* oldBody = body;
		btCompoundShape * mainShape = new btCompoundShape();
		
		if (body != nullptr)
		{
			if (body->getCollisionShape() != nullptr)
				delete body->getCollisionShape();

			PhysicsManager::getSingleton()->getWorld()->removeRigidBody(body);
			delete body;
		}

		if (motionState != nullptr)
			delete motionState;

		isTrigger = false;

		std::vector<Component*>& components = getGameObject()->getComponents();
		for (std::vector<Component*>::iterator it = components.begin(); it != components.end(); ++it)
		{
			if (dynamic_cast<Collider*>(*it))
			{
				Collider * col = (Collider*)(*it);
				if (!col->getEnabled())
					continue;

				if (col->getIsTrigger())
					isTrigger = true;

				Transform* trans = getGameObject()->getTransform();
				glm::vec3 scl = trans->getScale() * col->getScale();
				btCollisionShape * shape = col->getCollisionShape();

				if (shape != nullptr)
				{
					shape->setLocalScaling(btVector3(scl.x, scl.y, scl.z));

					btTransform shapeTransform;
					shapeTransform.setIdentity();
					shapeTransform.setOrigin(toBulletVec3(col->getOffset() * scl));
					shapeTransform.setRotation(toBulletQuat(col->getRotation()));
					
					mainShape->addChildShape(shapeTransform, shape);
				}
			}
		}
		
		centerOfMass.setIdentity();
		btVector3 localInertia(0, 0, 0);

		if (!isStatic && mass > 0 && mainShape->getNumChildShapes() > 0)
		{
			MeshRenderer* rend = (MeshRenderer*)getGameObject()->getComponent(MeshRenderer::COMPONENT_TYPE);
			if (rend != nullptr)
			{
				glm::vec3 center = rend->getBounds(false).getCenter();
				centerOfMass.setOrigin(toBulletVec3(center));
			}
			else
			{
				btVector3 principalInertia;
				btScalar* masses = new btScalar[mainShape->getNumChildShapes()];
				for (int j = 0; j < mainShape->getNumChildShapes(); j++)
				{
					masses[j] = mass / (float)mainShape->getNumChildShapes();
				}

				mainShape->calculatePrincipalAxisTransform(masses, centerOfMass, principalInertia);
			}

			for (int i = 0; i < mainShape->getNumChildShapes(); i++)
			{
				btTransform newChildTransform = centerOfMass.inverse() * mainShape->getChildTransform(i);
				mainShape->updateChildTransform(i, newChildTransform);
			}

			mainShape->calculateLocalInertia(mass, localInertia);
		}

		Transform* node = getGameObject()->getTransform();

		startTransform.setIdentity();
		startTransform.setOrigin(toBulletVec3(node->getPosition()));
		startTransform.setRotation(toBulletQuat(node->getRotation()));

		motionState = new btDefaultMotionState(startTransform * centerOfMass);

		btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motionState, mainShape, localInertia);
		body = new btRigidBody(rbInfo);

		if (isTrigger)
			body->setCollisionFlags(btCollisionObject::CF_NO_CONTACT_RESPONSE);
		
		body->setActivationState(DISABLE_DEACTIVATION);

		if (!isStatic)
			body->setMassProps(mass, localInertia);
		else
			body->setMassProps(0, localInertia);

		body->setLinearFactor(btVector3(!freezePositionX, !freezePositionY, !freezePositionZ));
		body->setAngularFactor(btVector3(!freezeRotationX, !freezeRotationY, !freezeRotationZ));
		
		body->updateInertiaTensor();
		body->clearForces();

		//body->setCcdMotionThreshold(5.0f);
		//body->setCcdSweptSphereRadius(0.50);
		//body->setContactProcessingThreshold(10.0f);

		PhysicsManager::getSingleton()->getWorld()->addRigidBody(body);

		body->setFriction(friction);
		body->setRestitution(bounciness);
		body->setDamping(linearDamping, angularDamping);

		updateVehicles(oldBody);
	}

	void RigidBody::updateVehicles(btRigidBody* oldBody)
	{
		auto objects = Engine::getSingleton()->getGameObjects();

		for (auto& obj : objects)
		{
			Vehicle* veh = (Vehicle*)obj->getComponent(Vehicle::COMPONENT_TYPE);
			if (veh == nullptr)
				continue;

			btRaycastVehicle* v = veh->getVehicle();
			if (v == nullptr)
				continue;

			if (v->getRigidBody() == oldBody)
				veh->onRefresh();
		}

		objects.clear();
	}

	void RigidBody::reload()
	{
		if (getGameObject() == nullptr)
			return;

		if (initialized)
		{
			if (getEnabled() && getGameObject()->getActive())
			{
				setupPhysics();
			}
		}
	}

	void RigidBody::update()
	{
		if (!getEnabled() || !getGameObject()->getActive())
			return;

		if (!physicsActive)
			return;

		if (body == nullptr)
			return;

		Transform* parent = getGameObject()->getTransform();

		if (parent != nullptr)
		{
			if (!isKinematic)
			{
				btTransform trans;

				if (body->getMotionState() != nullptr)
				{
					body->getMotionState()->getWorldTransform(trans);
					trans = trans * centerOfMass.inverse();
				}

				parent->setPosition(fromBulletVec3(trans.getOrigin()));
				parent->setRotation(fromBulletQuat(trans.getRotation()));
			}
			else
			{
				setPosition(parent->getPosition());
				setRotation(parent->getRotation());
			}
		}
	}

	float RigidBody::getMass()
	{
		return mass;
	}

	void RigidBody::setMass(float mass)
	{
		this->mass = mass;

		if (body != nullptr)
		{
			btVector3 localInertia(0, 0, 0);
			body->setMassProps(mass, localInertia);
			body->updateInertiaTensor();
		}
	}

	void RigidBody::setIsStatic(bool stat)
	{
		isStatic = stat;
		reload();
	}

	void RigidBody::setGravity(glm::vec3 value)
	{
		gravity = value;
		if (body != nullptr)
		{
			if (useOwnGravity)
				body->setGravity(btVector3(gravity.x, gravity.y, gravity.z));
		}
	}

	void RigidBody::setFriction(float value)
	{
		friction = value;
		if (body != nullptr)
		{
			body->setFriction(friction);
			body->updateInertiaTensor();
		}
	}

	void RigidBody::setLinearDamping(float value)
	{
		linearDamping = value;
		if (body != nullptr)
		{
			body->setDamping(linearDamping, angularDamping);
			body->updateInertiaTensor();
		}
	}

	void RigidBody::setAngularDamping(float value)
	{
		angularDamping = value;
		if (body != nullptr)
		{
			body->setDamping(linearDamping, angularDamping);
			body->updateInertiaTensor();
		}
	}

	void RigidBody::setBounciness(float value)
	{
		bounciness = value;
		if (body != nullptr)
		{
			body->setRestitution(bounciness);
			body->updateInertiaTensor();
		}
	}

	glm::vec3 RigidBody::getPosition()
	{
		if (!getEnabled() || !getGameObject()->getActive())
			return getGameObject()->getTransform()->getPosition();

		btTransform transform;
		if (!isKinematic)
			transform = body->getCenterOfMassTransform() * centerOfMass.inverse();
		else
		{
			btDefaultMotionState* mState = (btDefaultMotionState*)body->getMotionState();
			mState->getWorldTransform(transform);
			transform = transform * centerOfMass.inverse();
		}

		btVector3 p = transform.getOrigin();

		return fromBulletVec3(p);
	}

	glm::highp_quat RigidBody::getRotation()
	{
		if (!getEnabled() || !getGameObject()->getActive())
			return getGameObject()->getTransform()->getRotation();

		btTransform transform;
		if (!isKinematic)
			transform = body->getCenterOfMassTransform() * centerOfMass.inverse();
		else
		{
			btDefaultMotionState* mState = (btDefaultMotionState*)body->getMotionState();
			mState->getWorldTransform(transform);
			transform = transform * centerOfMass.inverse();
		}

		btQuaternion q = transform.getRotation();

		return fromBulletQuat(q);
	}

	void RigidBody::setPosition(glm::vec3 position)
	{
		if (body != nullptr)
		{
			if (!isKinematic)
			{
				btTransform transform = body->getCenterOfMassTransform() * centerOfMass.inverse();
				transform.setOrigin(btVector3(position.x, position.y, position.z));
				body->setCenterOfMassTransform(transform * centerOfMass);
			}
			else
			{
				btDefaultMotionState* mState = (btDefaultMotionState*)body->getMotionState();

				btTransform transform;
				mState->getWorldTransform(transform);
				transform = transform * centerOfMass.inverse();

				transform.setOrigin(btVector3(position.x, position.y, position.z));
				mState->setWorldTransform(transform * centerOfMass);
			}
		}
	}

	void RigidBody::setRotation(glm::highp_quat rotation)
	{
		if (body != nullptr)
		{
			if (!isKinematic)
			{
				btTransform transform;
				transform = body->getCenterOfMassTransform() * centerOfMass.inverse();
				btQuaternion r;
				r.setValue(rotation.x, rotation.y, rotation.z, rotation.w);
				transform.setRotation(r);
				body->setCenterOfMassTransform(transform * centerOfMass);
			}
			else
			{
				btDefaultMotionState* mState = (btDefaultMotionState*)body->getMotionState();

				btTransform transform;
				mState->getWorldTransform(transform);
				transform = transform * centerOfMass.inverse();

				btQuaternion r;
				r.setValue(rotation.x, rotation.y, rotation.z, rotation.w);
				transform.setRotation(r);

				mState->setWorldTransform(transform * centerOfMass);
			}
		}
	}

	void RigidBody::addForce(glm::vec3 force, glm::vec3 pos)
	{
		if (body != nullptr)
		{
			body->activate(true);
			body->applyForce(btVector3(force.x, force.y, force.z), btVector3(pos.x, pos.y, pos.z));
		}
	}

	void RigidBody::addTorque(glm::vec3 torque)
	{
		if (body != nullptr)
		{
			body->activate(true);
			body->applyTorque(btVector3(torque.x, torque.y, torque.z));
		}
	}

	glm::vec3 RigidBody::getLinearVelocity()
	{
		btVector3 vel = body->getLinearVelocity();

		return fromBulletVec3(vel);
	}

	void RigidBody::setLinearVelocity(glm::vec3 velocity)
	{
		body->setLinearVelocity(toBulletVec3(velocity));
	}

	glm::vec3 RigidBody::getAngularVelocity()
	{
		btVector3 vel = body->getAngularVelocity();

		return fromBulletVec3(vel);
	}

	void RigidBody::setAngularVelocity(glm::vec3 velocity)
	{
		body->setAngularVelocity(toBulletVec3(velocity));
	}

	void RigidBody::setFreezePositionX(bool value)
	{
		freezePositionX = value;
		if (body != nullptr)
			body->setLinearFactor(btVector3(!freezePositionX, !freezePositionY, !freezePositionZ));
	}

	void RigidBody::setFreezePositionY(bool value)
	{
		freezePositionY = value;
		if (body != nullptr)
			body->setLinearFactor(btVector3(!freezePositionX, !freezePositionY, !freezePositionZ));
	}

	void RigidBody::setFreezePositionZ(bool value)
	{
		freezePositionZ = value;
		if (body != nullptr)
			body->setLinearFactor(btVector3(!freezePositionX, !freezePositionY, !freezePositionZ));
	}

	void RigidBody::setFreezeRotationX(bool value)
	{
		freezeRotationX = value;
		if (body != nullptr)
			body->setAngularFactor(btVector3(!freezeRotationX, !freezeRotationY, !freezeRotationZ));
	}

	void RigidBody::setFreezeRotationY(bool value)
	{
		freezeRotationY = value;
		if (body != nullptr)
			body->setAngularFactor(btVector3(!freezeRotationX, !freezeRotationY, !freezeRotationZ));
	}

	void RigidBody::setFreezeRotationZ(bool value)
	{
		freezeRotationZ = value;
		if (body != nullptr)
			body->setAngularFactor(btVector3(!freezeRotationX, !freezeRotationY, !freezeRotationZ));
	}

	void RigidBody::setIsKinematic(bool value)
	{
		isKinematic = value;

		if (body != nullptr)
		{
			if (isKinematic)
			{
				body->setCollisionFlags(collisionFlags | btCollisionObject::CF_KINEMATIC_OBJECT);
			}
			else
			{
				body->setCollisionFlags(collisionFlags);
			}
		}
	}

	void RigidBody::onStateChanged()
	{
		if (getGameObject() == nullptr)
			return;

		bool active = getGameObject()->getActive() && getEnabled();

		if (Engine::getSingleton()->getIsRuntimeMode())
		{
			if (initialized)
			{
				if (active == false)
				{
					if (physicsActive)
					{
						if (body != nullptr)
						{
							PhysicsManager::getSingleton()->removeBody(this);
							PhysicsManager::getSingleton()->getWorld()->removeRigidBody(body);
						}

						physicsActive = false;
					}
				}
				else
				{
					if (!physicsActive)
					{
						setupPhysics();
					}
				}
			}
		}
	}

	void RigidBody::onAttach()
	{
		Component::onAttach();

		if (Engine::getSingleton()->getIsRuntimeMode())
			PhysicsManager::getSingleton()->addBody(this);

		onSceneLoaded();
	}

	void RigidBody::onDetach()
	{
		Component::onDetach();

		if (Engine::getSingleton()->getIsRuntimeMode())
		{
			PhysicsManager::getSingleton()->removeBody(this);
			PhysicsManager::getSingleton()->getWorld()->removeRigidBody(body);
			physicsActive = false;
		}
	}

	void RigidBody::onSceneLoaded()
	{
		if (Engine::getSingleton()->getIsRuntimeMode())
		{
			initialized = true;
			setupPhysics();
		}
	}

	void RigidBody::updateCollisionMask()
	{
		ProjectSettings* settings = Engine::getSingleton()->getSettings();
		auto& bodies = PhysicsManager::getSingleton()->getBodies();

		for (auto& body2 : bodies)
		{
			GameObject* obj2 = body2->getGameObject();
			btRigidBody* rb2 = body2->getNativeBody();

			int l1 = gameObject->getLayer();
			int l2 = obj2->getLayer();

			bool collision = settings->getCollisionMask(l1, l2) || settings->getCollisionMask(l2, l1);

			body->setIgnoreCollisionCheck(rb2, !collision);
			rb2->setIgnoreCollisionCheck(body, !collision);
		}
	}
}