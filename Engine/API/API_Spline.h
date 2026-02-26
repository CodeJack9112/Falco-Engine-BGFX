#pragma once

#include "API.h"

namespace GX
{
	class API_Spline
	{
	public:
		static void Register()
		{
			mono_add_internal_call("FalcoEngine.Spline::get_numPoints", (void*)getNumPoints);
			mono_add_internal_call("FalcoEngine.Spline::INTERNAL_addPoint", (void*)addPoint);
			mono_add_internal_call("FalcoEngine.Spline::INTERNAL_removePoint", (void*)removePoint);
			mono_add_internal_call("FalcoEngine.Spline::INTERNAL_getPoint", (void*)getPoint);
			mono_add_internal_call("FalcoEngine.Spline::INTERNAL_setPoint", (void*)setPoint);
			mono_add_internal_call("FalcoEngine.Spline::INTERNAL_getSplinePoint", (void*)getSplinePoint);
			mono_add_internal_call("FalcoEngine.Spline::INTERNAL_getSplineDirection", (void*)getSplineDirection);
		}

		static int getNumPoints(MonoObject* this_ptr);
		static void addPoint(MonoObject* this_ptr, API::Vector3* ref_val);
		static void removePoint(MonoObject* this_ptr, int index);
		static void getPoint(MonoObject* this_ptr, int index, API::Vector3* out_val);
		static void setPoint(MonoObject* this_ptr, int index, API::Vector3* ref_val);
		static void getSplinePoint(MonoObject* this_ptr, float t, API::Vector3* out_val);
		static void getSplineDirection(MonoObject* this_ptr, float t , API::Vector3* out_val);
	};
}