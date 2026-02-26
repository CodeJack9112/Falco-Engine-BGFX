#include "API_Text.h"

#include <iostream>

#include "../Core/Engine.h"
#include "../Core/APIManager.h"
#include "../Classes/StringConverter.h"
#include "../Assets/Font.h"

#include "../Components/Text.h"

namespace GX
{
	MonoString* API_Text::getText(MonoObject* this_ptr)
	{
		Text* uitext = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&uitext));

		std::string _str = CP_UNI(uitext->getText());
		MonoString* str = mono_string_new(APIManager::getSingleton()->getDomain(), _str.c_str());

		return str;
	}

	void API_Text::setText(MonoObject* this_ptr, MonoString* text)
	{
		Text* uitext = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&uitext));

		std::string _text = (const char*)mono_string_to_utf8((MonoString*)text);
		_text = CP_SYS(_text);

		uitext->setText(_text);
	}

	MonoObject* API_Text::getFont(MonoObject* this_ptr)
	{
		Text* uitext = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&uitext));

		if (uitext->getFont() != nullptr)
			return uitext->getFont()->getManagedObject();

		return nullptr;
	}

	void API_Text::setFont(MonoObject* this_ptr, MonoObject* font)
	{
		Text* uitext = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&uitext));

		Font* uifont = nullptr;
		mono_field_get_value(font, APIManager::getSingleton()->asset_ptr_field, reinterpret_cast<void*>(&uifont));

		uitext->setFont(uifont);
	}

	float API_Text::getFontSize(MonoObject* this_ptr)
	{
		Text* uitext = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&uitext));

		return uitext->getFontSize();
	}

	void API_Text::setFontSize(MonoObject* this_ptr, float size)
	{
		Text* uitext = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&uitext));

		uitext->setFontSize(size);
	}

	int API_Text::getHorizontalAlignment(MonoObject* this_ptr)
	{
		Text* uitext = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&uitext));

		return static_cast<int>(uitext->getHorizontalTextAlignment());
	}

	void API_Text::setHorizontalAlignment(MonoObject* this_ptr, int alignment)
	{
		Text* uitext = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&uitext));

		uitext->setHorizontalTextAlignment(static_cast<Text::TextHorizontalAlignment>(alignment));
	}

	int API_Text::getVerticalAlignment(MonoObject* this_ptr)
	{
		Text* uitext = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&uitext));

		return static_cast<int>(uitext->getVerticalTextAlignment());
	}

	void API_Text::setVerticalAlignment(MonoObject* this_ptr, int alignment)
	{
		Text* uitext = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&uitext));

		uitext->setVerticalTextAlignment(static_cast<Text::TextVerticalAlignment>(alignment));
	}
}