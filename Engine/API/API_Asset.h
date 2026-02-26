#pragma once

#include "API.h"

namespace GX
{
	class API_Asset
	{
	public:
		static void Register()
		{
			mono_add_internal_call("FalcoEngine.Asset::get_name", (void*)getName);
		}

		static MonoString* getName(MonoObject* this_ptr);
	};
}