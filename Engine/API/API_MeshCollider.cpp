#include "API_MeshCollider.h"

#include "../Core/APIManager.h"
#include "../Components/MeshCollider.h"

namespace GX
{
	bool API_MeshCollider::getConvex(MonoObject* this_ptr)
	{
		MeshCollider* component;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&component));

		return component->getConvex();
	}

	void API_MeshCollider::setConvex(MonoObject* this_ptr, bool value)
	{
		MeshCollider* component;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&component));

		component->setConvex(value);
	}
}