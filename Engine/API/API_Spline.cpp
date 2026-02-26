#include "../Core/APIManager.h"

#include "API_Spline.h"
#include "../Components/Spline.h"

namespace GX
{
	int API_Spline::getNumPoints(MonoObject* this_ptr)
	{
		Spline* component = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&component));

		if (component != nullptr)
			return component->getPoints().size();

		return 0;
	}

	void API_Spline::addPoint(MonoObject* this_ptr, API::Vector3* ref_val)
	{
		Spline* component = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&component));

		if (component != nullptr)
		{
			glm::vec3 val = glm::vec3(ref_val->x, ref_val->y, ref_val->z);
			component->getPoints().push_back(val);
		}
	}

	void API_Spline::removePoint(MonoObject* this_ptr, int index)
	{
		Spline* component = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&component));

		if (component != nullptr)
		{
			auto& points = component->getPoints();
			if (index >= 0 && index < points.size())
				points.erase(points.begin() + index);
		}
	}

	void API_Spline::getPoint(MonoObject* this_ptr, int index, API::Vector3* out_val)
	{
		Spline* component = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&component));

		glm::vec3 val = glm::vec3(0.0f);
		
		if (component != nullptr)
		{
			auto& points = component->getPoints();
			if (index >= 0 && index < points.size())
				val = points[index];
		}

		out_val->x = val.x;
		out_val->y = val.y;
		out_val->z = val.z;
	}

	void API_Spline::setPoint(MonoObject* this_ptr, int index, API::Vector3* ref_val)
	{
		Spline* component = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&component));

		if (component != nullptr)
		{
			auto& points = component->getPoints();
			if (index >= 0 && index < points.size())
				points[index] = glm::vec3(ref_val->x, ref_val->y, ref_val->z);
		}
	}

	void API_Spline::getSplinePoint(MonoObject* this_ptr, float t, API::Vector3* out_val)
	{
		Spline* component = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&component));

		glm::vec3 val = glm::vec3(0.0f);

		if (component != nullptr)
			val = component->getSplinePoint(t);

		out_val->x = val.x;
		out_val->y = val.y;
		out_val->z = val.z;
	}

	void API_Spline::getSplineDirection(MonoObject* this_ptr, float t, API::Vector3* out_val)
	{
		Spline* component = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&component));

		glm::vec3 val = glm::vec3(0.0f);

		if (component != nullptr)
			val = component->getSplineDirection(t);

		out_val->x = val.x;
		out_val->y = val.y;
		out_val->z = val.z;
	}
}