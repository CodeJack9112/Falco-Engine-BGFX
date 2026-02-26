#pragma once

#include "API.h"

namespace GX
{
	class API_NavMeshAgent
	{
	public:
		//Register methods
		static void Register()
		{
			mono_add_internal_call("FalcoEngine.NavMeshAgent::INTERNAL_getTargetPosition", (void*)getTargetPosition);
			mono_add_internal_call("FalcoEngine.NavMeshAgent::INTERNAL_setTargetPosition", (void*)setTargetPosition);
			mono_add_internal_call("FalcoEngine.NavMeshAgent::get_radius", (void*)getRadius);
			mono_add_internal_call("FalcoEngine.NavMeshAgent::set_radius", (void*)setRadius);
			mono_add_internal_call("FalcoEngine.NavMeshAgent::get_height", (void*)getHeight);
			mono_add_internal_call("FalcoEngine.NavMeshAgent::set_height", (void*)setHeight);
			mono_add_internal_call("FalcoEngine.NavMeshAgent::get_speed", (void*)getSpeed);
			mono_add_internal_call("FalcoEngine.NavMeshAgent::set_speed", (void*)setSpeed);
			mono_add_internal_call("FalcoEngine.NavMeshAgent::get_acceleration", (void*)getAcceleration);
			mono_add_internal_call("FalcoEngine.NavMeshAgent::set_acceleration", (void*)setAcceleration);
		}

	private:
		//Get target object
		static void getTargetPosition(MonoObject * this_ptr, API::Vector3 * out_position);

		//Set target object
		static void setTargetPosition(MonoObject * this_ptr, API::Vector3 * ref_position);

		//Get radius
		static float getRadius(MonoObject * this_ptr);

		//Set radius
		static void setRadius(MonoObject * this_ptr, float radius);

		//Get height
		static float getHeight(MonoObject * this_ptr);

		//Set height
		static void setHeight(MonoObject * this_ptr, float height);

		//Get speed
		static float getSpeed(MonoObject * this_ptr);

		//Set speed
		static void setSpeed(MonoObject * this_ptr, float speed);

		//Get acceleration
		static float getAcceleration(MonoObject * this_ptr);

		//Set acceleration
		static void setAcceleration(MonoObject * this_ptr, float acceleration);
	};
}