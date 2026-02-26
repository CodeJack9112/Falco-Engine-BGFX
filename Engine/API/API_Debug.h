#pragma once

#include "API.h"

namespace GX
{
	class API_Debug
	{
	public:
		//Register methods
		static void Register()
		{
			mono_add_internal_call("FalcoEngine.Debug::INTERNAL_log", (void*)log);
		}

	private:
		static void log(MonoString * stringVal);
	};
}