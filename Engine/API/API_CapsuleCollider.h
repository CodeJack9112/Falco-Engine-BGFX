#pragma once

#include "API.h"

namespace GX
{
	class API_CapsuleCollider
	{
	public:
		static void Register()
		{
			mono_add_internal_call("FalcoEngine.CapsuleCollider::get_radius", (void*)getRadius);
			mono_add_internal_call("FalcoEngine.CapsuleCollider::set_radius", (void*)setRadius);

			mono_add_internal_call("FalcoEngine.CapsuleCollider::get_height", (void*)getHeight);
			mono_add_internal_call("FalcoEngine.CapsuleCollider::set_height", (void*)setHeight);
		}

		//getRadius
		static float getRadius(MonoObject* this_ptr);

		//setRadius
		static void setRadius(MonoObject* this_ptr, float val);

		//getHeight
		static float getHeight(MonoObject* this_ptr);

		//setHeight
		static void setHeight(MonoObject* this_ptr, float val);
	};
}