#pragma once

#include "API.h"

namespace GX
{
	class API_LayerMask
	{
	public:
		//Register methods
		static void Register()
		{
			mono_add_internal_call("FalcoEngine.LayerMask::LayerToName", (void*)LayerToName);
			mono_add_internal_call("FalcoEngine.LayerMask::NameToLayer", (void*)NameToLayer);
		}

	private:
		static MonoString* LayerToName(int layer);
		static int NameToLayer(MonoString* name);
	};
}