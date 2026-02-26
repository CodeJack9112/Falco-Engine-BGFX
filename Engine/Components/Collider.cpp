#include "Collider.h"
#include "RigidBody.h"

#include "../Core/GameObject.h"

namespace GX
{
	Collider::~Collider()
	{
		if (collisionShape != nullptr)
		{
			delete collisionShape;
			collisionShape = nullptr;
		}

		reloadBody();
	}

	void Collider::reloadBody()
	{
		if (getGameObject() == nullptr)
			return;

		RigidBody* body = (RigidBody*)getGameObject()->getComponent(RigidBody::COMPONENT_TYPE);

		if (body != nullptr)
		{
			body->reload();
		}
	}

	void Collider::setCollisionShape(btCollisionShape* shape)
	{
		collisionShape = shape;

		reloadBody();
	}

	void Collider::setOffset(glm::vec3 value)
	{
		offset = value;

		reloadBody();
	}

	void Collider::setRotation(glm::highp_quat value)
	{
		rotation = value;

		reloadBody();
	}

	void Collider::setIsTrigger(bool value)
	{
		isTrigger = value;

		reloadBody();
	}

	void Collider::onAttach()
	{
		Component::onAttach();

		setCollisionShape(collisionShape);
	}

	void Collider::onStateChanged()
	{
		reloadBody();
	}

	void Collider::onSceneLoaded()
	{
		setCollisionShape(collisionShape);
	}
}