#include "API_FreeJoint.h"

#include "../Core/APIManager.h"
#include "../Core/Engine.h"
#include "../Core/GameObject.h"
#include "../Components/FreeJoint.h"
#include "../Components/RigidBody.h"

namespace GX
{
	void API_FreeJoint::getAnchor(MonoObject* this_ptr, API::Vector3* out_val)
	{
		FreeJoint* joint = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&joint));

		if (joint != nullptr)
		{
			glm::vec3 p = joint->getAnchor();

			out_val->x = p.x;
			out_val->y = p.y;
			out_val->z = p.z;
		}
	}

	void API_FreeJoint::setAnchor(MonoObject* this_ptr, API::Vector3* ref_val)
	{
		FreeJoint* joint = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&joint));

		if (joint != nullptr)
			joint->setAnchor(glm::vec3(ref_val->x, ref_val->y, ref_val->z));
	}

	void API_FreeJoint::getConnectedAnchor(MonoObject* this_ptr, API::Vector3* out_val)
	{
		FreeJoint* joint = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&joint));

		if (joint != nullptr)
		{
			glm::vec3 p = joint->getConnectedAnchor();

			out_val->x = p.x;
			out_val->y = p.y;
			out_val->z = p.z;
		}
	}

	void API_FreeJoint::setConnectedAnchor(MonoObject* this_ptr, API::Vector3* ref_val)
	{
		FreeJoint* joint = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&joint));

		if (joint != nullptr)
			joint->setConnectedAnchor(glm::vec3(ref_val->x, ref_val->y, ref_val->z));
	}

	void API_FreeJoint::getLimitMin(MonoObject* this_ptr, API::Vector3* out_val)
	{
		FreeJoint* joint = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&joint));

		if (joint != nullptr)
		{
			glm::vec3 p = joint->getLimitMin();

			out_val->x = p.x;
			out_val->y = p.y;
			out_val->z = p.z;
		}
	}

	void API_FreeJoint::setLimitMin(MonoObject* this_ptr, API::Vector3* ref_val)
	{
		FreeJoint* joint = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&joint));

		if (joint != nullptr)
			joint->setLimitMin(glm::vec3(ref_val->x, ref_val->y, ref_val->z));
	}

	void API_FreeJoint::getLimitMax(MonoObject* this_ptr, API::Vector3* out_val)
	{
		FreeJoint* joint = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&joint));

		if (joint != nullptr)
		{
			glm::vec3 p = joint->getLimitMax();

			out_val->x = p.x;
			out_val->y = p.y;
			out_val->z = p.z;
		}
	}

	void API_FreeJoint::setLimitMax(MonoObject* this_ptr, API::Vector3* ref_val)
	{
		FreeJoint* joint = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&joint));

		if (joint != nullptr)
			joint->setLimitMax(glm::vec3(ref_val->x, ref_val->y, ref_val->z));
	}

	bool API_FreeJoint::getLinkedBodiesCollision(MonoObject* this_ptr)
	{
		FreeJoint* joint = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&joint));

		if (joint != nullptr)
			return joint->getLinkedBodiesCollision();
	}

	void API_FreeJoint::setLinkedBodiesCollision(MonoObject* this_ptr, bool value)
	{
		FreeJoint* joint = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&joint));

		if (joint != nullptr)
			joint->setLinkedBodiesCollision(value);
	}

	MonoObject* API_FreeJoint::getConnectedObject(MonoObject* this_ptr)
	{
		FreeJoint* joint = nullptr;
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

	void API_FreeJoint::setConnectedObject(MonoObject* this_ptr, MonoObject* value)
	{
		FreeJoint* joint = nullptr;
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

	void API_FreeJoint::autoConfigureAnchors(MonoObject* this_ptr)
	{
		FreeJoint* joint = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&joint));

		if (joint != nullptr)
			joint->autoConfigureAnchors();
	}

	void API_FreeJoint::rebuild(MonoObject* this_ptr)
	{
		FreeJoint* joint = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&joint));

		if (joint != nullptr)
			joint->onRefresh();
	}
}