#pragma once

#include "API.h"

namespace GX
{
	class API_Screen
	{
	public:
		//Register methods
		static void Register()
		{
			mono_add_internal_call("FalcoEngine.Screen::get_width", (void*)getWidth);
			mono_add_internal_call("FalcoEngine.Screen::get_height", (void*)getHeight);
		}

	private:
		static int getWidth();
		static int getHeight();
	};
}