#include "API_RigidBody.h"

#include "../Core/GameObject.h"
#include "../Core/APIManager.h"
#include "../Components/RigidBody.h"

namespace GX
{
	void API_RigidBody::getPosition(MonoObject * this_ptr, API::Vector3 * out_pos)
	{
		RigidBody* body = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&body));

		if (body != nullptr)
		{
			glm::vec3 p = body->getPosition();

			out_pos->x = p.x;
			out_pos->y = p.y;
			out_pos->z = p.z;
		}
	}

	void API_RigidBody::setPosition(MonoObject * this_ptr, API::Vector3 * ref_pos)
	{
		RigidBody* body = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&body));

		if (body != nullptr)
			body->setPosition(glm::vec3(ref_pos->x, ref_pos->y, ref_pos->z));
	}

	void API_RigidBody::getRotation(MonoObject * this_ptr, API::Quaternion * out_rot)
	{
		RigidBody* body = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&body));

		if (body != nullptr)
		{
			glm::highp_quat q = body->getRotation();

			out_rot->x = q.x;
			out_rot->y = q.y;
			out_rot->z = q.z;
			out_rot->w = q.w;
		}
	}

	void API_RigidBody::setRotation(MonoObject * this_ptr, API::Quaternion * ref_rot)
	{
		RigidBody* body = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&body));

		if (body != nullptr)
			body->setRotation(glm::highp_quat(ref_rot->w, ref_rot->x, ref_rot->y, ref_rot->z));
	}

	void API_RigidBody::getLinearVelocity(MonoObject * this_ptr, API::Vector3 * out_vel)
	{
		RigidBody* body = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&body));

		if (body != nullptr)
		{
			glm::vec3 v = body->getLinearVelocity();

			out_vel->x = v.x;
			out_vel->y = v.y;
			out_vel->z = v.z;
		}
	}

	void API_RigidBody::setLinearVelocity(MonoObject * this_ptr, API::Vector3 * ref_vel)
	{
		RigidBody* body = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&body));

		if (body != nullptr)
			body->setLinearVelocity(glm::vec3(ref_vel->x, ref_vel->y, ref_vel->z));
	}

	void API_RigidBody::getAngularVelocity(MonoObject * this_ptr, API::Vector3 * out_vel)
	{
		RigidBody* body = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&body));

		if (body != nullptr)
		{
			glm::vec3 v = body->getAngularVelocity();

			out_vel->x = v.x;
			out_vel->y = v.y;
			out_vel->z = v.z;
		}
	}

	void API_RigidBody::setAngularVelocity(MonoObject * this_ptr, API::Vector3 * ref_vel)
	{
		RigidBody* body = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&body));

		if (body != nullptr)
			body->setAngularVelocity(glm::vec3(ref_vel->x, ref_vel->y, ref_vel->z));
	}

	void API_RigidBody::addForce(MonoObject * this_ptr, API::Vector3 * ref_force, API::Vector3 * ref_pos)
	{
		RigidBody* body = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&body));

		if (body != nullptr)
			body->addForce(glm::vec3(ref_force->x, ref_force->y, ref_force->z), glm::vec3(ref_pos->x, ref_pos->y, ref_pos->z));
	}

	void API_RigidBody::addTorque(MonoObject * this_ptr, API::Vector3 * ref_torque)
	{
		RigidBody* body = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&body));

		if (body != nullptr)
			body->addTorque(glm::vec3(ref_torque->x, ref_torque->y, ref_torque->z));
	}

	bool API_RigidBody::getIsKinematic(MonoObject * this_ptr)
	{
		RigidBody* body = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&body));

		if (body != nullptr)
			return body->getIsKinematic();

		return false;
	}

	void API_RigidBody::setIsKinematic(MonoObject * this_ptr, bool value)
	{
		RigidBody* body = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&body));

		if (body != nullptr)
			body->setIsKinematic(value);
	}

	bool API_RigidBody::getIsStatic(MonoObject* this_ptr)
	{
		RigidBody* body = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&body));

		if (body != nullptr)
			return body->getIsStatic();

		return false;
	}

	void API_RigidBody::setIsStatic(MonoObject* this_ptr, bool value)
	{
		RigidBody* body = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&body));

		if (body != nullptr)
			body->setIsStatic(value);
	}

	bool API_RigidBody::getIsTrigger(MonoObject* this_ptr)
	{
		RigidBody* body = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&body));

		if (body != nullptr)
			return body->getIsTrigger();

		return false;
	}

	float API_RigidBody::getMass(MonoObject* this_ptr)
	{
		RigidBody* body = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&body));

		if (body != nullptr)
			return body->getMass();

		return 0.0f;
	}

	void API_RigidBody::setMass(MonoObject* this_ptr, float value)
	{
		RigidBody* body = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&body));

		if (body != nullptr)
			body->setMass(value);
	}

	void API_RigidBody::setFreezePosition(MonoObject* this_ptr, bool freezeX, bool freezeY, bool freezeZ)
	{
		RigidBody* body = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&body));

		if (body != nullptr)
		{
			body->setFreezePositionX(freezeX);
			body->setFreezePositionY(freezeY);
			body->setFreezePositionZ(freezeZ);
		}
	}

	void API_RigidBody::setFreezeRotation(MonoObject* this_ptr, bool freezeX, bool freezeY, bool freezeZ)
	{
		RigidBody* body = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&body));

		if (body != nullptr)
		{
			body->setFreezeRotationX(freezeX);
			body->setFreezeRotationY(freezeY);
			body->setFreezeRotationZ(freezeZ);
		}
	}

	float API_RigidBody::getFriction(MonoObject* this_ptr)
	{
		RigidBody* body = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&body));

		if (body != nullptr)
			return body->getFriction();

		return 0.0f;
	}

	void API_RigidBody::setFriction(MonoObject* this_ptr, float value)
	{
		RigidBody* body = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&body));

		if (body != nullptr)
			body->setFriction(value);
	}

	float API_RigidBody::getBounciness(MonoObject* this_ptr)
	{
		RigidBody* body = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&body));

		if (body != nullptr)
			return body->getBounciness();

		return 0.0f;
	}

	void API_RigidBody::setBounciness(MonoObject* this_ptr, float value)
	{
		RigidBody* body = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&body));

		if (body != nullptr)
			body->setBounciness(value);
	}

	float API_RigidBody::getLinearDamping(MonoObject* this_ptr)
	{
		RigidBody* body = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&body));

		if (body != nullptr)
			return body->getLinearDamping();

		return 0.0f;
	}

	void API_RigidBody::setLinearDamping(MonoObject* this_ptr, float value)
	{
		RigidBody* body = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&body));

		if (body != nullptr)
			body->setLinearDamping(value);
	}

	float API_RigidBody::getAngularDamping(MonoObject* this_ptr)
	{
		RigidBody* body = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&body));

		if (body != nullptr)
			return body->getAngularDamping();

		return 0.0f;
	}

	void API_RigidBody::setAngularDamping(MonoObject* this_ptr, float value)
	{
		RigidBody* body = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&body));

		if (body != nullptr)
			body->setAngularDamping(value);
	}
}