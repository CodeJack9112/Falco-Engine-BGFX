#include "API_TextInput.h"

#include <iostream>

#include "../Core/Engine.h"
#include "../Core/APIManager.h"
#include "../Classes/StringConverter.h"

#include "../Components/TextInput.h"
#include "../Assets/Texture.h"
#include "../Assets/Font.h"

namespace GX
{
	MonoObject* API_TextInput::getNormalStateTexture(MonoObject* this_ptr)
	{
		TextInput* textInput = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&textInput));

		Texture* texture = textInput->getTextureNormal();

		if (texture != nullptr)
			return texture->getManagedObject();

		return nullptr;
	}

	void API_TextInput::setNormalStateTexture(MonoObject* this_ptr, MonoObject* texture)
	{
		TextInput* textInput = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&textInput));

		if (texture != nullptr)
		{
			Texture* _tex = nullptr;
			mono_field_get_value(texture, APIManager::getSingleton()->asset_ptr_field, reinterpret_cast<void*>(&_tex));

			textInput->setTextureNormal(_tex);
		}
		else
			textInput->setTextureNormal(nullptr);
	}

	MonoObject* API_TextInput::getHoverStateTexture(MonoObject* this_ptr)
	{
		TextInput* textInput = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&textInput));

		Texture* texture = textInput->getTextureHover();

		if (texture != nullptr)
			return texture->getManagedObject();

		return nullptr;
	}

	void API_TextInput::setHoverStateTexture(MonoObject* this_ptr, MonoObject* texture)
	{
		TextInput* textInput = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&textInput));

		if (texture != nullptr)
		{
			Texture* _tex = nullptr;
			mono_field_get_value(texture, APIManager::getSingleton()->asset_ptr_field, reinterpret_cast<void*>(&_tex));

			textInput->setTextureHover(_tex);
		}
		else
			textInput->setTextureHover(nullptr);
	}

	MonoObject* API_TextInput::getActiveStateTexture(MonoObject* this_ptr)
	{
		TextInput* textInput = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&textInput));

		Texture* texture = textInput->getTextureActive();

		if (texture != nullptr)
			return texture->getManagedObject();

		return nullptr;
	}

	void API_TextInput::setActiveStateTexture(MonoObject* this_ptr, MonoObject* texture)
	{
		TextInput* textInput = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&textInput));

		if (texture != nullptr)
		{
			Texture* _tex = nullptr;
			mono_field_get_value(texture, APIManager::getSingleton()->asset_ptr_field, reinterpret_cast<void*>(&_tex));

			textInput->setTextureActive(_tex);
		}
		else
			textInput->setTextureActive(nullptr);
	}

	MonoObject* API_TextInput::getDisabledStateTexture(MonoObject* this_ptr)
	{
		TextInput* textInput = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&textInput));

		Texture* texture = textInput->getTextureDisabled();

		if (texture != nullptr)
			return texture->getManagedObject();

		return nullptr;
	}

	void API_TextInput::setDisabledStateTexture(MonoObject* this_ptr, MonoObject* texture)
	{
		TextInput* textInput = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&textInput));

		if (texture != nullptr)
		{
			Texture* _tex = nullptr;
			mono_field_get_value(texture, APIManager::getSingleton()->asset_ptr_field, reinterpret_cast<void*>(&_tex));

			textInput->setTextureDisabled(_tex);
		}
		else
			textInput->setTextureDisabled(nullptr);
	}

	void API_TextInput::getHoverStateColor(MonoObject* this_ptr, API::Color* value)
	{
		TextInput* textInput = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&textInput));

		value->r = textInput->getColorHover().r();
		value->g = textInput->getColorHover().g();
		value->b = textInput->getColorHover().b();
		value->a = textInput->getColorHover().a();
	}

	void API_TextInput::setHoverStateColor(MonoObject* this_ptr, API::Color* value)
	{
		TextInput* textInput = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&textInput));

		textInput->setColorHover(Color(value->r, value->g, value->b, value->a));
	}

	void API_TextInput::getActiveStateColor(MonoObject* this_ptr, API::Color* value)
	{
		TextInput* textInput = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&textInput));

		value->r = textInput->getColorActive().r();
		value->g = textInput->getColorActive().g();
		value->b = textInput->getColorActive().b();
		value->a = textInput->getColorActive().a();
	}

	void API_TextInput::setActiveStateColor(MonoObject* this_ptr, API::Color* value)
	{
		TextInput* textInput = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&textInput));

		textInput->setColorActive(Color(value->r, value->g, value->b, value->a));
	}

	void API_TextInput::getDisabledStateColor(MonoObject* this_ptr, API::Color* value)
	{
		TextInput* textInput = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&textInput));

		value->r = textInput->getColorDisabled().r();
		value->g = textInput->getColorDisabled().g();
		value->b = textInput->getColorDisabled().b();
		value->a = textInput->getColorDisabled().a();
	}

	void API_TextInput::setDisabledStateColor(MonoObject* this_ptr, API::Color* value)
	{
		TextInput* textInput = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&textInput));

		textInput->setColorDisabled(Color(value->r, value->g, value->b, value->a));
	}

	bool API_TextInput::getInteractable(MonoObject* this_ptr)
	{
		TextInput* textInput = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&textInput));

		return textInput->getInteractable();
	}

	void API_TextInput::setInteractable(MonoObject* this_ptr, bool value)
	{
		TextInput* textInput = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&textInput));

		textInput->setInteractable(value);
	}

	MonoString* API_TextInput::getText(MonoObject* this_ptr)
	{
		TextInput* textInput = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&textInput));

		std::string _str = CP_UNI(textInput->getText());
		MonoString* str = mono_string_new(APIManager::getSingleton()->getDomain(), _str.c_str());

		return str;
	}

	void API_TextInput::setText(MonoObject* this_ptr, MonoString* text)
	{
		TextInput* textInput = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&textInput));

		std::string _text = (const char*)mono_string_to_utf8((MonoString*)text);
		_text = CP_SYS(_text);

		textInput->setText(_text);
	}

	MonoObject* API_TextInput::getFont(MonoObject* this_ptr)
	{
		TextInput* textInput = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&textInput));

		if (textInput->getFont() != nullptr)
			return textInput->getFont()->getManagedObject();

		return nullptr;
	}

	void API_TextInput::setFont(MonoObject* this_ptr, MonoObject* font)
	{
		TextInput* textInput = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&textInput));

		Font* uifont = nullptr;
		mono_field_get_value(font, APIManager::getSingleton()->asset_ptr_field, reinterpret_cast<void*>(&uifont));

		textInput->setFont(uifont);
	}

	float API_TextInput::getFontSize(MonoObject* this_ptr)
	{
		TextInput* textInput = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&textInput));

		return textInput->getFontSize();
	}

	void API_TextInput::setFontSize(MonoObject* this_ptr, float size)
	{
		TextInput* textInput = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&textInput));

		textInput->setFontSize(size);
	}

	int API_TextInput::getHorizontalAlignment(MonoObject* this_ptr)
	{
		TextInput* textInput = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&textInput));

		return static_cast<int>(textInput->getHorizontalTextAlignment());
	}

	void API_TextInput::setHorizontalAlignment(MonoObject* this_ptr, int alignment)
	{
		TextInput* textInput = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&textInput));

		textInput->setHorizontalTextAlignment(static_cast<TextInput::TextHorizontalAlignment>(alignment));
	}

	int API_TextInput::getVerticalAlignment(MonoObject* this_ptr)
	{
		TextInput* textInput = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&textInput));

		return static_cast<int>(textInput->getVerticalTextAlignment());
	}

	void API_TextInput::setVerticalAlignment(MonoObject* this_ptr, int alignment)
	{
		TextInput* textInput = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&textInput));

		textInput->setVerticalTextAlignment(static_cast<TextInput::TextVerticalAlignment>(alignment));
	}
}