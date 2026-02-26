#include "API_Prefab.h"

#include "../Core/Engine.h"
#include "../Assets/Scene.h"
#include "../Classes/StringConverter.h"
#include "../Components/RigidBody.h"
#include "../Components/Transform.h"
#include "../Core/APIManager.h"
#include "../Core/GameObject.h"

namespace GX
{
	MonoObject* API_Prefab::instantiate(MonoObject* this_ptr, API::Vector3 * position, API::Quaternion * rotation)
	{
		Prefab* prefab;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->asset_ptr_field, reinterpret_cast<void*>(&prefab));

		if (prefab == nullptr)
			return nullptr;

		glm::vec3 _pos = glm::vec3(position->x, position->y, position->z);
		glm::highp_quat _rot = glm::highp_quat(rotation->w, rotation->x, rotation->y, rotation->z);

		GameObject * node = Scene::loadPrefab(prefab, _pos, _rot);

		return node->getManagedObject();
	}
}