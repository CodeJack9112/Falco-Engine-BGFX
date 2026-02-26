#pragma once

#include "API.h"

namespace GX
{
	class API_TextInput
	{
	public:
		static void Register()
		{
			mono_add_internal_call("FalcoEngine.TextInput::get_normalStateTexture", (void*)getNormalStateTexture);
			mono_add_internal_call("FalcoEngine.TextInput::set_normalStateTexture", (void*)setNormalStateTexture);
			mono_add_internal_call("FalcoEngine.TextInput::get_hoverStateTexture", (void*)getHoverStateTexture);
			mono_add_internal_call("FalcoEngine.TextInput::set_hoverStateTexture", (void*)setHoverStateTexture);
			mono_add_internal_call("FalcoEngine.TextInput::get_activeStateTexture", (void*)getActiveStateTexture);
			mono_add_internal_call("FalcoEngine.TextInput::set_activeStateTexture", (void*)setActiveStateTexture);
			mono_add_internal_call("FalcoEngine.TextInput::get_disabledStateTexture", (void*)getDisabledStateTexture);
			mono_add_internal_call("FalcoEngine.TextInput::set_disabledStateTexture", (void*)setDisabledStateTexture);
			mono_add_internal_call("FalcoEngine.TextInput::INTERNAL_getHoverStateColor", (void*)getHoverStateColor);
			mono_add_internal_call("FalcoEngine.TextInput::INTERNAL_setHoverStateColor", (void*)setHoverStateColor);
			mono_add_internal_call("FalcoEngine.TextInput::INTERNAL_getActiveStateColor", (void*)getActiveStateColor);
			mono_add_internal_call("FalcoEngine.TextInput::INTERNAL_setActiveStateColor", (void*)setActiveStateColor);
			mono_add_internal_call("FalcoEngine.TextInput::INTERNAL_getDisabledStateColor", (void*)getDisabledStateColor);
			mono_add_internal_call("FalcoEngine.TextInput::INTERNAL_setDisabledStateColor", (void*)setDisabledStateColor);

			mono_add_internal_call("FalcoEngine.TextInput::get_interactable", (void*)getInteractable);
			mono_add_internal_call("FalcoEngine.TextInput::set_interactable", (void*)setInteractable);

			mono_add_internal_call("FalcoEngine.TextInput::get_text", (void*)getText);
			mono_add_internal_call("FalcoEngine.TextInput::set_text", (void*)setText);
			mono_add_internal_call("FalcoEngine.TextInput::get_font", (void*)getFont);
			mono_add_internal_call("FalcoEngine.TextInput::set_font", (void*)setFont);
			mono_add_internal_call("FalcoEngine.TextInput::get_fontSize", (void*)getFontSize);
			mono_add_internal_call("FalcoEngine.TextInput::set_fontSize", (void*)setFontSize);
			mono_add_internal_call("FalcoEngine.TextInput::INTERNAL_get_textHorizontalAlignment", (void*)getHorizontalAlignment);
			mono_add_internal_call("FalcoEngine.TextInput::INTERNAL_set_textHorizontalAlignment", (void*)setHorizontalAlignment);
			mono_add_internal_call("FalcoEngine.TextInput::INTERNAL_get_textVerticalAlignment", (void*)getVerticalAlignment);
			mono_add_internal_call("FalcoEngine.TextInput::INTERNAL_set_textVerticalAlignment", (void*)setVerticalAlignment);
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

		static MonoString* getText(MonoObject* this_ptr);
		static void setText(MonoObject* this_ptr, MonoString* text);

		static MonoObject* getFont(MonoObject* this_ptr);
		static void setFont(MonoObject* this_ptr, MonoObject* font);

		static float getFontSize(MonoObject* this_ptr);
		static void setFontSize(MonoObject* this_ptr, float size);

		static int getHorizontalAlignment(MonoObject* this_ptr);
		static void setHorizontalAlignment(MonoObject* this_ptr, int alignment);

		static int getVerticalAlignment(MonoObject* this_ptr);
		static void setVerticalAlignment(MonoObject* this_ptr, int alignment);
	};
}