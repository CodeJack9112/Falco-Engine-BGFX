#include "API_CapsuleCollider.h"

#include "../Core/APIManager.h"
#include "../Components/CapsuleCollider.h"

namespace GX
{
	float API_CapsuleCollider::getRadius(MonoObject* this_ptr)
	{
		CapsuleCollider* component;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&component));

		return component->getRadius();
	}

	void API_CapsuleCollider::setRadius(MonoObject* this_ptr, float val)
	{
		CapsuleCollider* component;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&component));

		component->setRadius(val);
	}

	float API_CapsuleCollider::getHeight(MonoObject* this_ptr)
	{
		CapsuleCollider* component;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&component));

		return component->getHeight();
	}

	void API_CapsuleCollider::setHeight(MonoObject* this_ptr, float val)
	{
		CapsuleCollider* component;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&component));

		component->setHeight(val);
	}
}