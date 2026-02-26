#include "API_ConeTwistJoint.h"

#include "../Core/APIManager.h"
#include "../Core/Engine.h"
#include "../Core/GameObject.h"
#include "../Components/ConeTwistJoint.h"
#include "../Components/RigidBody.h"

namespace GX
{
	void API_ConeTwistJoint::getAnchor(MonoObject* this_ptr, API::Vector3* out_val)
	{
		ConeTwistJoint* joint = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&joint));

		if (joint != nullptr)
		{
			glm::vec3 p = joint->getAnchor();

			out_val->x = p.x;
			out_val->y = p.y;
			out_val->z = p.z;
		}
	}

	void API_ConeTwistJoint::setAnchor(MonoObject* this_ptr, API::Vector3* ref_val)
	{
		ConeTwistJoint* joint = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&joint));

		if (joint != nullptr)
			joint->setAnchor(glm::vec3(ref_val->x, ref_val->y, ref_val->z));
	}

	void API_ConeTwistJoint::getConnectedAnchor(MonoObject* this_ptr, API::Vector3* out_val)
	{
		ConeTwistJoint* joint = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&joint));

		if (joint != nullptr)
		{
			glm::vec3 p = joint->getConnectedAnchor();

			out_val->x = p.x;
			out_val->y = p.y;
			out_val->z = p.z;
		}
	}

	void API_ConeTwistJoint::setConnectedAnchor(MonoObject* this_ptr, API::Vector3* ref_val)
	{
		ConeTwistJoint* joint = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&joint));

		if (joint != nullptr)
			joint->setConnectedAnchor(glm::vec3(ref_val->x, ref_val->y, ref_val->z));
	}

	void API_ConeTwistJoint::getLimits(MonoObject* this_ptr, API::Vector3* out_val)
	{
		ConeTwistJoint* joint = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&joint));

		if (joint != nullptr)
		{
			glm::vec3 p = joint->getLimits();

			out_val->x = p.x;
			out_val->y = p.y;
			out_val->z = p.z;
		}
	}

	void API_ConeTwistJoint::setLimits(MonoObject* this_ptr, API::Vector3* ref_val)
	{
		ConeTwistJoint* joint = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&joint));

		if (joint != nullptr)
			joint->setLimits(glm::vec3(ref_val->x, ref_val->y, ref_val->z));
	}

	bool API_ConeTwistJoint::getLinkedBodiesCollision(MonoObject* this_ptr)
	{
		ConeTwistJoint* joint = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&joint));

		if (joint != nullptr)
			return joint->getLinkedBodiesCollision();
	}

	void API_ConeTwistJoint::setLinkedBodiesCollision(MonoObject* this_ptr, bool value)
	{
		ConeTwistJoint* joint = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&joint));

		if (joint != nullptr)
			joint->setLinkedBodiesCollision(value);
	}

	MonoObject* API_ConeTwistJoint::getConnectedObject(MonoObject* this_ptr)
	{
		ConeTwistJoint* joint = nullptr;
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

	void API_ConeTwistJoint::setConnectedObject(MonoObject* this_ptr, MonoObject* value)
	{
		ConeTwistJoint* joint = nullptr;
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

	void API_ConeTwistJoint::autoConfigureAnchors(MonoObject* this_ptr)
	{
		ConeTwistJoint* joint = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&joint));

		if (joint != nullptr)
			joint->autoConfigureAnchors();
	}

	void API_ConeTwistJoint::rebuild(MonoObject* this_ptr)
	{
		ConeTwistJoint* joint = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&joint));

		if (joint != nullptr)
			joint->onRefresh();
	}
}