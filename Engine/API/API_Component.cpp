#include "API_Component.h"

#include "../Core/APIManager.h"
#include "../Core/GameObject.h"
#include "../Components/Component.h"

namespace GX
{
	bool API_Component::getEnabled(MonoObject * this_ptr)
	{
		Component * component = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&component));

		if (component != nullptr)
			return component->getEnabled();

		return false;
	}

	void API_Component::setEnabled(MonoObject * this_ptr, bool enabled)
	{
		Component * component = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&component));

		if (component != nullptr)
			component->setEnabled(enabled);
	}

	MonoObject* API_Component::getGameObject(MonoObject* this_ptr)
	{
		Component* component = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&component));

		if (component != nullptr)
			return component->getGameObject()->getManagedObject();

		return nullptr;
	}

	bool API_Component::isUiComponent(MonoObject* this_ptr)
	{
		Component* component = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&component));

		if (component != nullptr)
			return component->isUiComponent();

		return false;
	}
}