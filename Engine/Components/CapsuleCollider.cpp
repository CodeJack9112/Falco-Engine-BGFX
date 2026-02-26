#include "CapsuleCollider.h"

#include "RigidBody.h"
#include "../Core/APIManager.h"

#include "../Bullet/include/btBulletCollisionCommon.h"
#include "../Bullet/include/btBulletDynamicsCommon.h"

namespace GX
{
	std::string CapsuleCollider::COMPONENT_TYPE = "CapsuleCollider";

	CapsuleCollider::CapsuleCollider() : Collider(APIManager::getSingleton()->capsulecollider_class)
	{
		collisionShape = new btCapsuleShape(radius, height);
	}

	CapsuleCollider::~CapsuleCollider()
	{
	}

	std::string CapsuleCollider::getComponentType()
	{
		return COMPONENT_TYPE;
	}

	Component* CapsuleCollider::onClone()
	{
		CapsuleCollider* newComponent = new CapsuleCollider();
		newComponent->enabled = enabled;
		newComponent->offset = offset;
		newComponent->rotation = rotation;
		newComponent->isTrigger = isTrigger;
		newComponent->radius = radius;
		newComponent->height = height;

		return newComponent;
	}

	void CapsuleCollider::setRadius(float r)
	{
		radius = r;

		delete collisionShape;
		collisionShape = new btCapsuleShape(radius, height);
		setCollisionShape(collisionShape);
	}

	void CapsuleCollider::setHeight(float h)
	{
		height = h;

		delete collisionShape;
		collisionShape = new btCapsuleShape(radius, height);
		setCollisionShape(collisionShape);
	}
}