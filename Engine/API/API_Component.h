#pragma once

#include "API.h"

namespace GX
{
	class API_Component
	{
	public:
		static void Register()
		{
			mono_add_internal_call("FalcoEngine.Component::get_enabled", (void*)getEnabled);
			mono_add_internal_call("FalcoEngine.Component::set_enabled", (void*)setEnabled);
			mono_add_internal_call("FalcoEngine.Component::get_gameObject", (void*)getGameObject);
			mono_add_internal_call("FalcoEngine.Component::get_isUiComponent", (void*)isUiComponent);
		}

		//Get enabled
		static bool getEnabled(MonoObject * this_ptr);

		//Get enabled
		static void setEnabled(MonoObject * this_ptr, bool enabled);

		//Get GameObject
		static MonoObject* getGameObject(MonoObject* this_ptr);

		//Is UI component
		static bool isUiComponent(MonoObject* this_ptr);
	};
}