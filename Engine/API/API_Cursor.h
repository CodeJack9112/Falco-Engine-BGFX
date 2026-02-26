#pragma once

#include "API.h"

namespace GX
{
	class API_Cursor
	{
	public:
		//Register methods
		static void Register()
		{
			mono_add_internal_call("FalcoEngine.Cursor::get_visible", (void*)getVisible);
			mono_add_internal_call("FalcoEngine.Cursor::set_visible", (void*)setVisible);
			mono_add_internal_call("FalcoEngine.Cursor::get_locked", (void*)getLocked);
			mono_add_internal_call("FalcoEngine.Cursor::set_locked", (void*)setLocked);
		}

	private:
		static bool getVisible();
		static void setVisible(bool value);
		static bool getLocked();
		static void setLocked(bool locked);
	};
}