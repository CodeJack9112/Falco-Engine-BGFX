#include "API_Vehicle.h"

#include "../Core/APIManager.h"
#include "../Components/RigidBody.h"
#include "../Components/Vehicle.h"

#include "../Math/Mathf.h"

namespace GX
{
	int API_Vehicle::getNumWheels(MonoObject* this_ptr)
	{
		Vehicle* vehicle = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&vehicle));

		if (vehicle != nullptr)
			return vehicle->getWheels().size();

		return 0;
	}

	float API_Vehicle::getSpeedKMH(MonoObject* this_ptr)
	{
		Vehicle* vehicle = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&vehicle));

		if (vehicle != nullptr && vehicle->getVehicle() != nullptr)
			return vehicle->getVehicle()->getCurrentSpeedKmHour();

		return 0;
	}

	float API_Vehicle::getSteering(MonoObject* this_ptr, int wheel)
	{
		Vehicle* vehicle = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&vehicle));

		if (vehicle != nullptr)
		{
			if (wheel < vehicle->getWheels().size())
				return Mathf::fRad2Deg * vehicle->getVehicle()->getSteeringValue(wheel);
			else
				return 0;
		}

		return 0;
	}

	void API_Vehicle::setSteering(MonoObject* this_ptr, float angle, int wheel)
	{
		Vehicle* vehicle = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&vehicle));

		if (vehicle != nullptr)
		{
			if (wheel < vehicle->getWheels().size())
				vehicle->getVehicle()->setSteeringValue(Mathf::fDeg2Rad * angle, wheel);
		}
	}

	void API_Vehicle::setBreak(MonoObject* this_ptr, float value, int wheel)
	{
		Vehicle* vehicle = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&vehicle));

		if (vehicle != nullptr)
		{
			if (wheel < vehicle->getWheels().size())
				vehicle->getVehicle()->setBrake(value, wheel);
		}
	}

	void API_Vehicle::applyEngineForce(MonoObject* this_ptr, float value, int wheel)
	{
		Vehicle* vehicle = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&vehicle));

		if (vehicle != nullptr)
		{
			if (wheel < vehicle->getWheels().size())
				vehicle->getVehicle()->applyEngineForce(value, wheel);
		}
	}
}