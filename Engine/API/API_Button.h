#pragma once

#include "API.h"

namespace GX
{
	class API_Button
	{
	public:
		static void Register()
		{
			mono_add_internal_call("FalcoEngine.Button::get_normalStateTexture", (void*)getNormalStateTexture);
			mono_add_internal_call("FalcoEngine.Button::set_normalStateTexture", (void*)setNormalStateTexture);
			mono_add_internal_call("FalcoEngine.Button::get_hoverStateTexture", (void*)getHoverStateTexture);
			mono_add_internal_call("FalcoEngine.Button::set_hoverStateTexture", (void*)setHoverStateTexture);
			mono_add_internal_call("FalcoEngine.Button::get_activeStateTexture", (void*)getActiveStateTexture);
			mono_add_internal_call("FalcoEngine.Button::set_activeStateTexture", (void*)setActiveStateTexture);
			mono_add_internal_call("FalcoEngine.Button::get_disabledStateTexture", (void*)getDisabledStateTexture);
			mono_add_internal_call("FalcoEngine.Button::set_disabledStateTexture", (void*)setDisabledStateTexture);
			mono_add_internal_call("FalcoEngine.Button::INTERNAL_getHoverStateColor", (void*)getHoverStateColor);
			mono_add_internal_call("FalcoEngine.Button::INTERNAL_setHoverStateColor", (void*)setHoverStateColor);
			mono_add_internal_call("FalcoEngine.Button::INTERNAL_getActiveStateColor", (void*)getActiveStateColor);
			mono_add_internal_call("FalcoEngine.Button::INTERNAL_setActiveStateColor", (void*)setActiveStateColor);
			mono_add_internal_call("FalcoEngine.Button::INTERNAL_getDisabledStateColor", (void*)getDisabledStateColor);
			mono_add_internal_call("FalcoEngine.Button::INTERNAL_setDisabledStateColor", (void*)setDisabledStateColor);

			mono_add_internal_call("FalcoEngine.Button::get_interactable", (void*)getInteractable);
			mono_add_internal_call("FalcoEngine.Button::set_interactable", (void*)setInteractable);
		}

	private:
		static MonoObject* getNormalStateTexture(MonoObject* this_ptr);
		static void setNormalStateTexture(MonoObject* this_ptr, MonoObject* texture);

		static MonoObject* getHoverStateTexture(MonoObject* this_ptr);
		static void setHoverStateTexture(MonoObject* this_ptr, MonoObject* texture);

		static MonoObject* getActiveStateTexture(MonoObject* this_ptr);
		static void setActiveStateTexture(MonoObject* this_ptr, MonoObject* texture);

		static MonoObject* getDisabledStateTexture(MonoObject* this_ptr);
		static void setDisabledStateTexture(MonoObject* this_ptr, MonoObject* texture);

		//------------

		static void getHoverStateColor(MonoObject* this_ptr, API::Color* value);
		static void setHoverStateColor(MonoObject* this_ptr, API::Color* value);

		static void getActiveStateColor(MonoObject* this_ptr, API::Color* value);
		static void setActiveStateColor(MonoObject* this_ptr, API::Color* value);

		static void getDisabledStateColor(MonoObject* this_ptr, API::Color* value);
		static void setDisabledStateColor(MonoObject* this_ptr, API::Color* value);

		//------------

		static bool getInteractable(MonoObject* this_ptr);
		static void setInteractable(MonoObject* this_ptr, bool value);
	};
}