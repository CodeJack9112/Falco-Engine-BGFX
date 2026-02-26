#pragma once

#include "API.h"

namespace GX
{
	class API_Prefab
	{
	public:
		//Register methods
		static void Register()
		{
			mono_add_internal_call("FalcoEngine.Prefab::INTERNAL_instantiate", (void*)instantiate);
		}

		static MonoObject* instantiate(MonoObject * this_ptr, API::Vector3 * position, API::Quaternion * rotation);
	};
}