#include "API_FixedJoint.h"

#include "../Core/APIManager.h"
#include "../Core/Engine.h"
#include "../Core/GameObject.h"
#include "../Components/FixedJoint.h"
#include "../Components/RigidBody.h"

namespace GX
{
	void API_FixedJoint::getAnchor(MonoObject* this_ptr, API::Vector3* out_val)
	{
		FixedJoint* joint = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&joint));

		if (joint != nullptr)
		{
			glm::vec3 p = joint->getAnchor();

			out_val->x = p.x;
			out_val->y = p.y;
			out_val->z = p.z;
		}
	}

	void API_FixedJoint::setAnchor(MonoObject* this_ptr, API::Vector3* ref_val)
	{
		FixedJoint* joint = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&joint));

		if (joint != nullptr)
			joint->setAnchor(glm::vec3(ref_val->x, ref_val->y, ref_val->z));
	}

	void API_FixedJoint::getConnectedAnchor(MonoObject* this_ptr, API::Vector3* out_val)
	{
		FixedJoint* joint = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&joint));

		if (joint != nullptr)
		{
			glm::vec3 p = joint->getConnectedAnchor();

			out_val->x = p.x;
			out_val->y = p.y;
			out_val->z = p.z;
		}
	}

	void API_FixedJoint::setConnectedAnchor(MonoObject* this_ptr, API::Vector3* ref_val)
	{
		FixedJoint* joint = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&joint));

		if (joint != nullptr)
			joint->setConnectedAnchor(glm::vec3(ref_val->x, ref_val->y, ref_val->z));
	}

	bool API_FixedJoint::getLinkedBodiesCollision(MonoObject* this_ptr)
	{
		FixedJoint* joint = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&joint));

		if (joint != nullptr)
			return joint->getLinkedBodiesCollision();
	}

	void API_FixedJoint::setLinkedBodiesCollision(MonoObject* this_ptr, bool value)
	{
		FixedJoint* joint = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&joint));

		if (joint != nullptr)
			joint->setLinkedBodiesCollision(value);
	}

	MonoObject* API_FixedJoint::getConnectedObject(MonoObject* this_ptr)
	{
		FixedJoint* joint = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&joint));

		if (joint != nullptr)
		{
			std::string guid = joint->getConnectedObjectGuid();
			GameObject* obj = Engine::getSingleton()->getGameObject(guid);
			if (obj == nullptr)
				return nullptr;

			RigidBody* body = obj->getRigidBody();
			if (body == nullptr)
				return nullptr;

			return body->getManagedObject();
		}
	}

	void API_FixedJoint::setConnectedObject(MonoObject* this_ptr, MonoObject* value)
	{
		FixedJoint* joint = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&joint));

		if (joint != nullptr)
		{
			if (value == nullptr)
			{
				joint->setConnectedObjectGuid("");
			}
			else
			{
				RigidBody* body = nullptr;
				mono_field_get_value(value, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&body));

				if (body == nullptr)
					return;

				GameObject* obj = body->getGameObject();
				if (obj == nullptr)
					return;

				joint->setConnectedObjectGuid(obj->getGuid());
			}
		}
	}

	void API_FixedJoint::autoConfigureAnchors(MonoObject* this_ptr)
	{
		FixedJoint* joint = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&joint));

		if (joint != nullptr)
			joint->autoConfigureAnchors();
	}

	void API_FixedJoint::rebuild(MonoObject* this_ptr)
	{
		FixedJoint* joint = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&joint));

		if (joint != nullptr)
			joint->onRefresh();
	}
}