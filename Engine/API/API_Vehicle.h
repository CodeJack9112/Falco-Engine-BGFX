#pragma once

#include "API.h"

namespace GX
{
	class API_Vehicle
	{
	public:
		//Register methods
		static void Register()
		{
			mono_add_internal_call("FalcoEngine.Vehicle::get_numWheels", (void*)getNumWheels);
			mono_add_internal_call("FalcoEngine.Vehicle::get_speedKMH", (void*)getSpeedKMH);
			mono_add_internal_call("FalcoEngine.Vehicle::INTERNAL_getSteering", (void*)getSteering);
			mono_add_internal_call("FalcoEngine.Vehicle::INTERNAL_setSteering", (void*)setSteering);
			mono_add_internal_call("FalcoEngine.Vehicle::INTERNAL_setBreak", (void*)setBreak);
			mono_add_internal_call("FalcoEngine.Vehicle::INTERNAL_applyEngineForce", (void*)applyEngineForce);
		}

	private:
		static int getNumWheels(MonoObject* this_ptr);
		static float getSpeedKMH(MonoObject* this_ptr);
		static float getSteering(MonoObject* this_ptr, int wheel);
		static void setSteering(MonoObject* this_ptr, float angle, int wheel);
		static void setBreak(MonoObject* this_ptr, float value, int wheel);
		static void applyEngineForce(MonoObject* this_ptr, float value, int wheel);
	};
}