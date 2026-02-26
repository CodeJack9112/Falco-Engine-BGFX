#pragma once

#include "API.h"

namespace GX
{
	class API_MeshCollider
	{
	public:
		static void Register()
		{
			mono_add_internal_call("FalcoEngine.MeshCollider::get_convex", (void*)getConvex);
			mono_add_internal_call("FalcoEngine.MeshCollider::set_convex", (void*)setConvex);
		}

		//Get enabled
		static bool getConvex(MonoObject* this_ptr);

		//Get enabled
		static void setConvex(MonoObject* this_ptr, bool value);
	};
}