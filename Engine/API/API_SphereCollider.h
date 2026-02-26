#pragma once

#include "API.h"

namespace GX
{
	class API_SphereCollider
	{
	public:
		static void Register()
		{
			mono_add_internal_call("FalcoEngine.SphereCollider::get_radius", (void*)getRadius);
			mono_add_internal_call("FalcoEngine.SphereCollider::set_radius", (void*)setRadius);
		}

		//getRadius
		static float getRadius(MonoObject* this_ptr);

		//setRadius
		static void setRadius(MonoObject* this_ptr, float val);
	};
}