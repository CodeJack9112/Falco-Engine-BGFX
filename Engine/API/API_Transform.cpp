#include "API_Transform.h"

#include "../Core/GameObject.h"
#include "../Core/APIManager.h"
#include "../Components/Transform.h"
#include "../Components/RigidBody.h"
#include "../Classes/StringConverter.h"
#include "../Math/Mathf.h"

#include "../glm/vec3.hpp"

namespace GX
{
	void API_Transform::getPosition(MonoObject * this_ptr, API::Vector3 * out_pos)
	{
		Transform* node = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&node));

		RigidBody* body = (RigidBody*)node->getGameObject()->getRigidBody();

		if (body != nullptr)
		{
			glm::vec3 p = body->getPosition();

			out_pos->x = p.x;
			out_pos->y = p.y;
			out_pos->z = p.z;
		}
		else
		{
			out_pos->x = node->getPosition().x;
			out_pos->y = node->getPosition().y;
			out_pos->z = node->getPosition().z;
		}
	}

	void API_Transform::setPosition(MonoObject * this_ptr, API::Vector3* ref_pos)
	{
		Transform* node = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&node));

		RigidBody* body = (RigidBody*)node->getGameObject()->getRigidBody();

		if (body != nullptr)
		{
			node->setPosition(glm::vec3(ref_pos->x, ref_pos->y, ref_pos->z));
			body->setPosition(glm::vec3(ref_pos->x, ref_pos->y, ref_pos->z));
		}
		else
		{
			node->setPosition(glm::vec3(ref_pos->x, ref_pos->y, ref_pos->z));
		}
	}

	void API_Transform::getScale(MonoObject * this_ptr, API::Vector3* out_scale)
	{
		Transform* node = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&node));

		out_scale->x = node->getScale().x;
		out_scale->y = node->getScale().y;
		out_scale->z = node->getScale().z;
	}

	void API_Transform::setScale(MonoObject * this_ptr, API::Vector3* ref_scale)
	{
		Transform* node = nullptr;

		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&node));
		node->setScale(glm::vec3(ref_scale->x, ref_scale->y, ref_scale->z));
	}

	void API_Transform::getRotation(MonoObject * this_ptr, API::Quaternion * out_rot)
	{
		Transform* node = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&node));

		RigidBody* body = (RigidBody*)node->getGameObject()->getRigidBody();

		if (body != nullptr)
		{
			glm::highp_quat q = body->getRotation();

			out_rot->x = q.x;
			out_rot->y = q.y;
			out_rot->z = q.z;
			out_rot->w = q.w;
		}
		else
		{
			glm::highp_quat q = node->getRotation();

			out_rot->x = q.x;
			out_rot->y = q.y;
			out_rot->z = q.z;
			out_rot->w = q.w;
		}
	}

	void API_Transform::setRotation(MonoObject * this_ptr, API::Quaternion * ref_rot)
	{
		Transform* node = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&node));

		RigidBody* body = (RigidBody*)node->getGameObject()->getRigidBody();

		if (body != nullptr)
		{
			node->setRotation(glm::highp_quat(ref_rot->w, ref_rot->x, ref_rot->y, ref_rot->z));
			body->setRotation(glm::highp_quat(ref_rot->w, ref_rot->x, ref_rot->y, ref_rot->z));
		}
		else
		{
			node->setRotation(glm::highp_quat(ref_rot->w, ref_rot->x, ref_rot->y, ref_rot->z));
		}
	}

	void API_Transform::getLocalPosition(MonoObject * this_ptr, API::Vector3* out_pos)
	{
		Transform* node = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&node));

		out_pos->x = node->getLocalPosition().x;
		out_pos->y = node->getLocalPosition().y;
		out_pos->z = node->getLocalPosition().z;
	}

	void API_Transform::setLocalPosition(MonoObject * this_ptr, API::Vector3* ref_pos)
	{
		Transform* node = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&node));

		node->setLocalPosition(glm::vec3(ref_pos->x, ref_pos->y, ref_pos->z));
	}

	void API_Transform::getLocalRotation(MonoObject * this_ptr, API::Quaternion * out_rot)
	{
		Transform* node = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&node));

		glm::highp_quat q = node->getLocalRotation();

		out_rot->x = q.x;
		out_rot->y = q.y;
		out_rot->z = q.z;
		out_rot->w = q.w;
	}

	void API_Transform::setLocalRotation(MonoObject * this_ptr, API::Quaternion * ref_rot)
	{
		Transform* node = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&node));

		RigidBody* body = (RigidBody*)node->getGameObject()->getRigidBody();

		if (body != nullptr)
		{
			node->setLocalRotation(glm::highp_quat(ref_rot->w, ref_rot->x, ref_rot->y, ref_rot->z));
			body->setRotation(node->getRotation());
		}
		else
		{
			node->setLocalRotation(glm::highp_quat(ref_rot->w, ref_rot->x, ref_rot->y, ref_rot->z));
		}
	}

	void API_Transform::getLocalScale(MonoObject* this_ptr, API::Vector3* out_val)
	{
		Transform* node = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&node));

		out_val->x = node->getLocalScale().x;
		out_val->y = node->getLocalScale().y;
		out_val->z = node->getLocalScale().z;
	}

	void API_Transform::setLocalScale(MonoObject* this_ptr, API::Vector3* ref_val)
	{
		Transform* node = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&node));

		node->setLocalScale(glm::vec3(ref_val->x, ref_val->y, ref_val->z));
	}

	void API_Transform::getForward(MonoObject* this_ptr, API::Vector3* out_val)
	{
		Transform* node = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&node));

		glm::vec3 vec = node->getForward();

		out_val->x = vec.x;
		out_val->y = vec.y;
		out_val->z = vec.z;
	}

	void API_Transform::getUp(MonoObject* this_ptr, API::Vector3* out_val)
	{
		Transform* node = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&node));

		glm::vec3 vec = node->getUp();

		out_val->x = vec.x;
		out_val->y = vec.y;
		out_val->z = vec.z;
	}

	void API_Transform::getLeft(MonoObject* this_ptr, API::Vector3* out_val)
	{
		Transform* node = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&node));

		glm::vec3 vec = node->getRight();

		out_val->x = vec.x;
		out_val->y = vec.y;
		out_val->z = vec.z;
	}

	void API_Transform::getRight(MonoObject* this_ptr, API::Vector3* out_val)
	{
		Transform* node = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&node));

		glm::vec3 vec = -node->getRight();

		out_val->x = vec.x;
		out_val->y = vec.y;
		out_val->z = vec.z;
	}

	int API_Transform::getChildCount(MonoObject * this_ptr)
	{
		Transform* node = nullptr;

		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&node));

		return node->getChildren().size();
	}

	MonoObject * API_Transform::getParent(MonoObject * this_ptr)
	{
		Transform* node = nullptr;

		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&node));

		if (node->getParent() != nullptr)
			return node->getParent()->getManagedObject();
		
		return nullptr;
	}

	void API_Transform::setParent(MonoObject * this_ptr, MonoObject * parent)
	{
		Transform* node = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&node));

		Transform* parentNode = nullptr;
		mono_field_get_value(parent, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&parentNode));

		node->setParent(parentNode);
	}

	MonoObject* API_Transform::getChild(MonoObject* this_ptr, int index)
	{
		Transform* node = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&node));

		Transform* child = node->getChild(index);
		if (child != nullptr)
			return child->getManagedObject();

		return nullptr;
	}

	MonoObject* API_Transform::findChild(MonoObject* this_ptr, MonoString* name)
	{
		Transform* node = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&node));

		MonoObject* ret = nullptr;

		std::vector<Transform*>& children = node->getChildren();

		for (auto it = children.begin(); it != children.end(); ++it)
		{
			Transform* child = *it;

			std::string _name = CP_SYS(mono_string_to_utf8(name));
			if (child->getGameObject()->getName() == _name)
			{
				ret = child->getManagedObject();
				break;
			}
		}

		return ret;
	}

	void API_Transform::transformDirection(MonoObject* this_ptr, API::Vector3* ref_direction, API::Vector3* out_ret)
	{
		Transform* node = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&node));

		glm::vec3 dir = glm::vec3(ref_direction->x, ref_direction->y, ref_direction->z);

		glm::vec3 result = Mathf::transformDirection(node, dir);

		out_ret->x = result.x;
		out_ret->y = result.y;
		out_ret->z = result.z;
	}

	void API_Transform::inverseTransformDirection(MonoObject* this_ptr, API::Vector3* ref_direction, API::Vector3* out_ret)
	{
		Transform* node = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&node));

		glm::vec3 dir = glm::vec3(ref_direction->x, ref_direction->y, ref_direction->z);

		glm::vec3 result = Mathf::inverseTransformDirection(node, dir);

		out_ret->x = result.x;
		out_ret->y = result.y;
		out_ret->z = result.z;
	}

	void API_Transform::transformPoint(MonoObject* this_ptr, API::Vector3* ref_position, API::Vector3* out_ret)
	{
		Transform* node = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&node));

		glm::vec3 pos = glm::vec3(ref_position->x, ref_position->y, ref_position->z);

		glm::vec3 result = Mathf::transformPoint(node, pos);

		out_ret->x = result.x;
		out_ret->y = result.y;
		out_ret->z = result.z;
	}

	void API_Transform::inverseTransformPoint(MonoObject* this_ptr, API::Vector3* ref_position, API::Vector3* out_ret)
	{
		Transform* node = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&node));

		glm::vec3 pos = glm::vec3(ref_position->x, ref_position->y, ref_position->z);

		glm::vec3 result = Mathf::inverseTransformPoint(node, pos);

		out_ret->x = result.x;
		out_ret->y = result.y;
		out_ret->z = result.z;
	}
}