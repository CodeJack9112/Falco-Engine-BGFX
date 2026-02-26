#include "API_Light.h"

#include "../Components/Light.h"

#include "../Core/APIManager.h"

namespace GX
{
	int API_Light::getType(MonoObject* this_ptr)
	{
		Light* light = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&light));

		int type = static_cast<int>(light->getLightType());

		return type;
	}

	void API_Light::setType(MonoObject* this_ptr, int value)
	{
		Light* light = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&light));

		LightType type = static_cast<LightType>(value);

		light->setLightType(type);
	}

	void API_Light::getColor(MonoObject* this_ptr, API::Color* out_color)
	{
		Light* light = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&light));

		Color color = light->getColor();

		out_color->r = color[0];
		out_color->g = color[1];
		out_color->b = color[2];
		out_color->a = color[3];
	}

	void API_Light::setColor(MonoObject* this_ptr, API::Color* ref_color)
	{
		Light* light = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&light));

		Color color = Color(ref_color->r, ref_color->g, ref_color->b, ref_color->a);

		light->setColor(color);
	}

	float API_Light::getIntensity(MonoObject* this_ptr)
	{
		Light* light = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&light));

		return light->getIntensity();
	}

	void API_Light::setIntensity(MonoObject* this_ptr, float value)
	{
		Light* light = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&light));

		light->setIntensity(value);
	}

	float API_Light::getRadius(MonoObject* this_ptr)
	{
		Light* light = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&light));

		return light->getRadius();
	}

	void API_Light::setRadius(MonoObject* this_ptr, float value)
	{
		Light* light = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&light));

		light->setRadius(value);
	}

	float API_Light::getInnerRadius(MonoObject* this_ptr)
	{
		Light* light = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&light));

		return light->getInnerRadius();
	}

	void API_Light::setInnerRadius(MonoObject* this_ptr, float value)
	{
		Light* light = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&light));

		light->setInnerRadius(value);
	}

	float API_Light::getOuterRadius(MonoObject* this_ptr)
	{
		Light* light = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&light));

		return light->getOuterRadius();
	}

	void API_Light::setOuterRadius(MonoObject* this_ptr, float value)
	{
		Light* light = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&light));

		light->setOuterRadius(value);
	}

	float API_Light::getBias(MonoObject* this_ptr)
	{
		Light* light = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&light));

		return light->getBias();
	}

	void API_Light::setBias(MonoObject* this_ptr, float value)
	{
		Light* light = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&light));

		light->setBias(value);
	}

	bool API_Light::getCastShadows(MonoObject* this_ptr)
	{
		Light* light = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&light));

		return light->getCastShadows();
	}

	void API_Light::setCastShadows(MonoObject* this_ptr, bool value)
	{
		Light* light = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&light));

		light->setCastShadows(value);
	}
}