#include "API_NavMeshAgent.h"

#include "../Core/Engine.h"
#include "../Core/APIManager.h"
#include "../Core/GameObject.h"
#include "../Core/NavigationManager.h"
#include "../Components/NavMeshAgent.h"

namespace GX
{
	void API_NavMeshAgent::getTargetPosition(MonoObject * this_ptr, API::Vector3 * out_position)
	{
		NavMeshAgent* agent = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&agent));

		if (agent != nullptr)
		{	
			out_position->x = agent->getTargetPosition().x;
			out_position->y = agent->getTargetPosition().y;
			out_position->z = agent->getTargetPosition().z;
		}
	}

	void API_NavMeshAgent::setTargetPosition(MonoObject * this_ptr, API::Vector3 * ref_position)
	{
		NavMeshAgent* agent = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&agent));

		if (agent != nullptr)
		{
			agent->setTargetPosition(glm::vec3(ref_position->x, ref_position->y, ref_position->z));
		}
	}

	float API_NavMeshAgent::getRadius(MonoObject * this_ptr)
	{
		NavMeshAgent* agent = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&agent));

		if (agent != nullptr)
		{
			return agent->getRadius();
		}

		return 0;
	}

	void API_NavMeshAgent::setRadius(MonoObject * this_ptr, float radius)
	{
		NavMeshAgent* agent = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&agent));

		if (agent != nullptr)
		{
			agent->setRadius(radius);
		}
	}

	float API_NavMeshAgent::getHeight(MonoObject * this_ptr)
	{
		NavMeshAgent* agent = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&agent));

		if (agent != nullptr)
		{
			return agent->getHeight();
		}

		return 0;
	}

	void API_NavMeshAgent::setHeight(MonoObject * this_ptr, float height)
	{
		NavMeshAgent* agent = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&agent));

		if (agent != nullptr)
		{
			agent->setHeight(height);
		}
	}

	float API_NavMeshAgent::getSpeed(MonoObject * this_ptr)
	{
		NavMeshAgent* agent = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&agent));

		if (agent != nullptr)
		{
			return agent->getSpeed();
		}

		return 0;
	}

	void API_NavMeshAgent::setSpeed(MonoObject * this_ptr, float speed)
	{
		NavMeshAgent* agent = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&agent));

		if (agent != nullptr)
		{
			agent->setSpeed(speed);
		}
	}

	float API_NavMeshAgent::getAcceleration(MonoObject * this_ptr)
	{
		NavMeshAgent* agent = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&agent));

		if (agent != nullptr)
		{
			return agent->getAcceleration();
		}

		return 0;
	}

	void API_NavMeshAgent::setAcceleration(MonoObject * this_ptr, float acceleration)
	{
		NavMeshAgent* agent = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&agent));

		if (agent != nullptr)
		{
			agent->setAcceleration(acceleration);
		}
	}
}