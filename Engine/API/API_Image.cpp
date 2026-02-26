#include "API_Image.h"

#include <iostream>

#include "../Core/Engine.h"
#include "../Core/APIManager.h"
#include "../Classes/StringConverter.h"
#include "../Assets/Texture.h"

#include "../Components/Image.h"

namespace GX
{
	MonoObject* API_Image::getTexture(MonoObject* this_ptr)
	{
		Image* image = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&image));

		Texture* texture = image->getTexture();

		if (texture != nullptr)
			return texture->getManagedObject();

		return nullptr;
	}

	void API_Image::setTexture(MonoObject* this_ptr, MonoObject* texture)
	{
		Image* image = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&image));

		if (texture != nullptr)
		{
			Texture* _tex = nullptr;
			mono_field_get_value(texture, APIManager::getSingleton()->asset_ptr_field, reinterpret_cast<void*>(&_tex));

			image->setTexture(_tex);
		}
		else
			image->setTexture(nullptr);
	}
}