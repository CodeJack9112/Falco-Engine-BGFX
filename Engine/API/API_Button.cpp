#include "API_Button.h"

#include <iostream>

#include "../Core/Engine.h"
#include "../Core/APIManager.h"
#include "../Classes/StringConverter.h"

#include "../Components/Button.h"
#include "../Assets/Texture.h"

namespace GX
{
	MonoObject* API_Button::getNormalStateTexture(MonoObject* this_ptr)
	{
		Button* button = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&button));

		Texture* texture = button->getTextureNormal();

		if (texture != nullptr)
			return texture->getManagedObject();

		return nullptr;
	}

	void API_Button::setNormalStateTexture(MonoObject* this_ptr, MonoObject* texture)
	{
		Button* button = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&button));

		if (texture != nullptr)
		{
			Texture* _tex = nullptr;
			mono_field_get_value(texture, APIManager::getSingleton()->asset_ptr_field, reinterpret_cast<void*>(&_tex));

			button->setTextureNormal(_tex);
		}
		else
			button->setTextureNormal(nullptr);
	}

	MonoObject* API_Button::getHoverStateTexture(MonoObject* this_ptr)
	{
		Button* button = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&button));

		Texture* texture = button->getTextureHover();

		if (texture != nullptr)
			return texture->getManagedObject();

		return nullptr;
	}

	void API_Button::setHoverStateTexture(MonoObject* this_ptr, MonoObject* texture)
	{
		Button* button = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&button));

		if (texture != nullptr)
		{
			Texture* _tex = nullptr;
			mono_field_get_value(texture, APIManager::getSingleton()->asset_ptr_field, reinterpret_cast<void*>(&_tex));

			button->setTextureHover(_tex);
		}
		else
			button->setTextureHover(nullptr);
	}

	MonoObject* API_Button::getActiveStateTexture(MonoObject* this_ptr)
	{
		Button* button = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&button));

		Texture* texture = button->getTextureActive();

		if (texture != nullptr)
			return texture->getManagedObject();

		return nullptr;
	}

	void API_Button::setActiveStateTexture(MonoObject* this_ptr, MonoObject* texture)
	{
		Button* button = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&button));

		if (texture != nullptr)
		{
			Texture* _tex = nullptr;
			mono_field_get_value(texture, APIManager::getSingleton()->asset_ptr_field, reinterpret_cast<void*>(&_tex));

			button->setTextureActive(_tex);
		}
		else
			button->setTextureActive(nullptr);
	}

	MonoObject* API_Button::getDisabledStateTexture(MonoObject* this_ptr)
	{
		Button* button = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&button));

		Texture* texture = button->getTextureDisabled();

		if (texture != nullptr)
			return texture->getManagedObject();

		return nullptr;
	}

	void API_Button::setDisabledStateTexture(MonoObject* this_ptr, MonoObject* texture)
	{
		Button* button = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&button));

		if (texture != nullptr)
		{
			Texture* _tex = nullptr;
			mono_field_get_value(texture, APIManager::getSingleton()->asset_ptr_field, reinterpret_cast<void*>(&_tex));

			button->setTextureDisabled(_tex);
		}
		else
			button->setTextureDisabled(nullptr);
	}

	void API_Button::getHoverStateColor(MonoObject* this_ptr, API::Color* value)
	{
		Button* button = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&button));

		value->r = button->getColorHover().r();
		value->g = button->getColorHover().g();
		value->b = button->getColorHover().b();
		value->a = button->getColorHover().a();
	}

	void API_Button::setHoverStateColor(MonoObject* this_ptr, API::Color* value)
	{
		Button* button = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&button));

		button->setColorHover(Color(value->r, value->g, value->b, value->a));
	}

	void API_Button::getActiveStateColor(MonoObject* this_ptr, API::Color* value)
	{
		Button* button = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&button));

		value->r = button->getColorActive().r();
		value->g = button->getColorActive().g();
		value->b = button->getColorActive().b();
		value->a = button->getColorActive().a();
	}

	void API_Button::setActiveStateColor(MonoObject* this_ptr, API::Color* value)
	{
		Button* button = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&button));

		button->setColorActive(Color(value->r, value->g, value->b, value->a));
	}

	void API_Button::getDisabledStateColor(MonoObject* this_ptr, API::Color* value)
	{
		Button* button = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&button));

		value->r = button->getColorDisabled().r();
		value->g = button->getColorDisabled().g();
		value->b = button->getColorDisabled().b();
		value->a = button->getColorDisabled().a();
	}

	void API_Button::setDisabledStateColor(MonoObject* this_ptr, API::Color* value)
	{
		Button* button = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&button));

		button->setColorDisabled(Color(value->r, value->g, value->b, value->a));
	}

	bool API_Button::getInteractable(MonoObject* this_ptr)
	{
		Button* button = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&button));

		return button->getInteractable();
	}

	void API_Button::setInteractable(MonoObject* this_ptr, bool value)
	{
		Button* button = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&button));

		button->setInteractable(value);
	}
}