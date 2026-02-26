#pragma once

#include "API.h"

namespace GX
{
	class API_Collider
	{
	public:
		static void Register()
		{
			mono_add_internal_call("FalcoEngine.Collider::INTERNAL_get_offset", (void*)getOffset);
			mono_add_internal_call("FalcoEngine.Collider::INTERNAL_set_offset", (void*)setOffset);

			mono_add_internal_call("FalcoEngine.Collider::INTERNAL_get_rotation", (void*)getRotation);
			mono_add_internal_call("FalcoEngine.Collider::INTERNAL_set_rotation", (void*)setRotation);
		}

		//getOffset
		static void getOffset(MonoObject* this_ptr, API::Vector3 * out_val);

		//setOffset
		static void setOffset(MonoObject* this_ptr, API::Vector3* ref_val);

		//getRotation
		static void getRotation(MonoObject* this_ptr, API::Quaternion * out_val);

		//setRotation
		static void setRotation(MonoObject* this_ptr, API::Quaternion* ref_val);
	};
}