#include "SphereCollider.h"

#include "RigidBody.h"
#include "../Core/APIManager.h"

#include "../Bullet/include/btBulletCollisionCommon.h"
#include "../Bullet/include/btBulletDynamicsCommon.h"

namespace GX
{
	std::string SphereCollider::COMPONENT_TYPE = "SphereCollider";

	SphereCollider::SphereCollider() : Collider(APIManager::getSingleton()->spherecollider_class)
	{
		collisionShape = new btSphereShape(radius);
	}

	SphereCollider::~SphereCollider()
	{
	}

	std::string SphereCollider::getComponentType()
	{
		return COMPONENT_TYPE;
	}

	Component* SphereCollider::onClone()
	{
		SphereCollider* newComponent = new SphereCollider();
		newComponent->enabled = enabled;
		newComponent->offset = offset;
		newComponent->rotation = rotation;
		newComponent->isTrigger = isTrigger;
		newComponent->radius = radius;

		return newComponent;
	}

	void SphereCollider::setRadius(float value)
	{
		radius = value;

		delete collisionShape;
		collisionShape = new btSphereShape(radius);
		setCollisionShape(collisionShape);
	}
}