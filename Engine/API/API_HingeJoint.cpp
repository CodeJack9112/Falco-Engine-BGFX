#include "API_HingeJoint.h"

#include "../Core/APIManager.h"
#include "../Core/Engine.h"
#include "../Core/GameObject.h"
#include "../Components/HingeJoint.h"
#include "../Components/RigidBody.h"

namespace GX
{
	void API_HingeJoint::getAnchor(MonoObject* this_ptr, API::Vector3* out_val)
	{
		HingeJoint* joint = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&joint));

		if (joint != nullptr)
		{
			glm::vec3 p = joint->getAnchor();

			out_val->x = p.x;
			out_val->y = p.y;
			out_val->z = p.z;
		}
	}

	void API_HingeJoint::setAnchor(MonoObject* this_ptr, API::Vector3* ref_val)
	{
		HingeJoint* joint = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&joint));

		if (joint != nullptr)
			joint->setAnchor(glm::vec3(ref_val->x, ref_val->y, ref_val->z));
	}

	void API_HingeJoint::getConnectedAnchor(MonoObject* this_ptr, API::Vector3* out_val)
	{
		HingeJoint* joint = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&joint));

		if (joint != nullptr)
		{
			glm::vec3 p = joint->getConnectedAnchor();

			out_val->x = p.x;
			out_val->y = p.y;
			out_val->z = p.z;
		}
	}

	void API_HingeJoint::setConnectedAnchor(MonoObject* this_ptr, API::Vector3* ref_val)
	{
		HingeJoint* joint = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&joint));

		if (joint != nullptr)
			joint->setConnectedAnchor(glm::vec3(ref_val->x, ref_val->y, ref_val->z));
	}

	void API_HingeJoint::getAxis(MonoObject* this_ptr, API::Vector3* out_val)
	{
		HingeJoint* joint = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&joint));

		if (joint != nullptr)
		{
			glm::vec3 p = joint->getAxis();

			out_val->x = p.x;
			out_val->y = p.y;
			out_val->z = p.z;
		}
	}

	void API_HingeJoint::setAxis(MonoObject* this_ptr, API::Vector3* ref_val)
	{
		HingeJoint* joint = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&joint));

		if (joint != nullptr)
			joint->setAxis(glm::vec3(ref_val->x, ref_val->y, ref_val->z));
	}

	float API_HingeJoint::getLimitMin(MonoObject* this_ptr)
	{
		HingeJoint* joint = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&joint));

		if (joint != nullptr)
			return joint->getLimitMin();
	}

	void API_HingeJoint::setLimitMin(MonoObject* this_ptr, float value)
	{
		HingeJoint* joint = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&joint));

		if (joint != nullptr)
			joint->setLimitMin(value);
	}

	float API_HingeJoint::getLimitMax(MonoObject* this_ptr)
	{
		HingeJoint* joint = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&joint));

		if (joint != nullptr)
			return joint->getLimitMax();
	}

	void API_HingeJoint::setLimitMax(MonoObject* this_ptr, float value)
	{
		HingeJoint* joint = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&joint));

		if (joint != nullptr)
			joint->setLimitMax(value);
	}

	bool API_HingeJoint::getLinkedBodiesCollision(MonoObject* this_ptr)
	{
		HingeJoint* joint = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&joint));

		if (joint != nullptr)
			return joint->getLinkedBodiesCollision();
	}

	void API_HingeJoint::setLinkedBodiesCollision(MonoObject* this_ptr, bool value)
	{
		HingeJoint* joint = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&joint));

		if (joint != nullptr)
			joint->setLinkedBodiesCollision(value);
	}

	MonoObject* API_HingeJoint::getConnectedObject(MonoObject* this_ptr)
	{
		HingeJoint* joint = nullptr;
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

	void API_HingeJoint::setConnectedObject(MonoObject* this_ptr, MonoObject* value)
	{
		HingeJoint* joint = nullptr;
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

	void API_HingeJoint::autoConfigureAnchors(MonoObject* this_ptr)
	{
		HingeJoint* joint = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&joint));

		if (joint != nullptr)
			joint->autoConfigureAnchors();
	}

	void API_HingeJoint::rebuild(MonoObject* this_ptr)
	{
		HingeJoint* joint = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&joint));

		if (joint != nullptr)
			joint->onRefresh();
	}
}