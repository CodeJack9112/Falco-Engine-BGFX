#pragma once

#include "API.h"

namespace GX
{
	class API_BoxCollider
	{
	public:
		static void Register()
		{
			mono_add_internal_call("FalcoEngine.BoxCollider::INTERNAL_get_size", (void*)getSize);
			mono_add_internal_call("FalcoEngine.BoxCollider::INTERNAL_set_size", (void*)setSize);
		}

		//getSize
		static void getSize(MonoObject* this_ptr, API::Vector3* out_val);

		//setSize
		static void setSize(MonoObject* this_ptr, API::Vector3* ref_val);
	};
}