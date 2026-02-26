#include "API_SphereCollider.h"

#include "../Core/APIManager.h"
#include "../Components/SphereCollider.h"

namespace GX
{
	float API_SphereCollider::getRadius(MonoObject* this_ptr)
	{
		SphereCollider* component;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&component));

		return component->getRadius();
	}

	void API_SphereCollider::setRadius(MonoObject* this_ptr, float val)
	{
		SphereCollider* component;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&component));

		component->setRadius(val);
	}
}