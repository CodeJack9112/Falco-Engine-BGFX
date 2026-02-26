#include "BoxCollider.h"

#include "RigidBody.h"
#include "../Core/APIManager.h"

#include "../Bullet/include/btBulletCollisionCommon.h"
#include "../Bullet/include/btBulletDynamicsCommon.h"

namespace GX
{
	std::string BoxCollider::COMPONENT_TYPE = "BoxCollider";

	BoxCollider::BoxCollider() : Collider(APIManager::getSingleton()->boxcollider_class)
	{
		collisionShape = new btBoxShape(btVector3(1, 1, 1));
	}

	BoxCollider::~BoxCollider()
	{
	}

	std::string BoxCollider::getComponentType()
	{
		return COMPONENT_TYPE;
	}

	Component* BoxCollider::onClone()
	{
		BoxCollider* newComponent = new BoxCollider();
		newComponent->enabled = enabled;
		newComponent->offset = offset;
		newComponent->rotation = rotation;
		newComponent->isTrigger = isTrigger;
		newComponent->boxSize = boxSize;

		return newComponent;
	}

	void BoxCollider::setBoxSize(glm::vec3 value)
	{
		boxSize = value;

		delete collisionShape;
		collisionShape = new btBoxShape(btVector3(value.x, value.y, value.z));
		setCollisionShape(collisionShape);
	}
}