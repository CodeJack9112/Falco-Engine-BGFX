#pragma once

#include "API.h"

namespace GX
{
	class API_Canvas
	{
	public:
		static void Register()
		{
			mono_add_internal_call("FalcoEngine.Canvas::get_scale", (void*)getScale);
		}

	private:
		static float getScale(MonoObject* this_ptr);
	};
}