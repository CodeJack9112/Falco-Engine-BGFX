#include "API_BoxCollider.h"

#include "../Core/APIManager.h"
#include "../Components/BoxCollider.h"

namespace GX
{
	void API_BoxCollider::getSize(MonoObject* this_ptr, API::Vector3* out_val)
	{
		BoxCollider* component;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&component));

		glm::vec3 size = component->getBoxSize();

		out_val->x = size.x;
		out_val->y = size.y;
		out_val->z = size.z;
	}

	void API_BoxCollider::setSize(MonoObject* this_ptr, API::Vector3* ref_val)
	{
		BoxCollider* component;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&component));

		component->setBoxSize(glm::vec3(ref_val->x, ref_val->y, ref_val->z));
	}
}