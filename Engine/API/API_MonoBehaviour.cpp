#include "API_MonoBehaviour.h"

#include "../../Mono/include/mono/metadata/reflection.h"

#include "../Core/Engine.h"
#include "../Core/GameObject.h"
#include "../Core/APIManager.h"
#include "../Components/Component.h"
#include "../Components/Transform.h"
#include "../Components/RigidBody.h"
#include "../Components/Animation.h"
#include "../Components/AudioSource.h"
#include "../Components/NavMeshAgent.h"
#include "../Components/MonoScript.h"

#include "../Classes/StringConverter.h"

namespace GX
{
	MonoString * API_MonoBehaviour::getName(MonoObject * this_ptr)
	{
		MonoScript* node = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&node));

		if (node != nullptr)
		{
			GameObject* obj = node->getGameObject();
			MonoString* str = mono_string_new(APIManager::getSingleton()->getDomain(), CP_UNI(obj->getName()).c_str());

			return str;
		}

		return nullptr;
	}

	MonoString * API_MonoBehaviour::getUniqueName(MonoObject* this_ptr)
	{
		MonoScript* node = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&node));

		if (node != nullptr)
		{
			GameObject* obj = node->getGameObject();
			MonoString* str = mono_string_new(APIManager::getSingleton()->getDomain(), CP_UNI(obj->getGuid()).c_str());

			return str;
		}

		return nullptr;
	}

	MonoObject * API_MonoBehaviour::getTransform(MonoObject * this_ptr)
	{
		MonoScript * node = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&node));

		if (node != nullptr)
		{
			GameObject* obj = node->getGameObject();
			Transform* trans = obj->getTransform();
			
			if (trans != nullptr)
				return trans->getManagedObject();
		}

		return nullptr;
	}

	MonoObject * API_MonoBehaviour::getRigidbody(MonoObject * this_ptr)
	{
		MonoScript * node = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&node));

		if (node != nullptr)
		{
			GameObject* obj = node->getGameObject();
			RigidBody* body = (RigidBody*)obj->getComponent(RigidBody::COMPONENT_TYPE);

			if (body != nullptr)
				return body->getManagedObject();
		}

		return nullptr;
	}

	MonoObject * API_MonoBehaviour::getGameObject(MonoObject * this_ptr)
	{
		MonoScript * node = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&node));

		if (node != nullptr)
		{
			GameObject* obj = node->getGameObject();
			return obj->getManagedObject();
		}

		return nullptr;
	}

	MonoObject * API_MonoBehaviour::getAnimation(MonoObject * this_ptr)
	{
		MonoScript * node = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&node));

		if (node != nullptr)
		{
			GameObject* obj = node->getGameObject();
			Animation* animList = ((Animation*)obj->getComponent(Animation::COMPONENT_TYPE));
			if (animList != nullptr)
				return animList->getManagedObject();
		}

		return nullptr;
	}

	MonoObject * API_MonoBehaviour::getAudioSource(MonoObject * this_ptr)
	{
		MonoScript * node = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&node));

		if (node != nullptr)
		{
			GameObject* obj = node->getGameObject();
			AudioSource* audio = ((AudioSource*)obj->getComponent(AudioSource::COMPONENT_TYPE));
			
			if (audio != nullptr)
				return audio->getManagedObject();
		}

		return nullptr;
	}

	MonoObject * API_MonoBehaviour::getNavMeshAgent(MonoObject * this_ptr)
	{
		MonoScript * node = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&node));

		if (node != nullptr)
		{
			GameObject* obj = node->getGameObject();
			NavMeshAgent* agent = ((NavMeshAgent*)obj->getComponent(NavMeshAgent::COMPONENT_TYPE));
			
			if (agent != nullptr)
				return agent->getManagedObject();
		}

		return nullptr;
	}

	MonoArray* API_MonoBehaviour::getComponents(MonoObject* this_ptr)
	{
		MonoScript* node = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&node));

		if (node != nullptr)
		{
			std::vector<Component*>& components = node->getGameObject()->getComponents();

			MonoArray* arr = mono_array_new(APIManager::getSingleton()->getDomain(), APIManager::getSingleton()->component_class, components.size());
			for (int i = 0; i < components.size(); ++i)
				mono_array_setref(arr, i, components[i]->managedObject);

			return arr;
		}

		return nullptr;
	}

	MonoObject * API_MonoBehaviour::getComponentT(MonoObject * this_ptr, MonoReflectionType * type)
	{
		MonoScript * node = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&node));

		if (node != nullptr)
		{
			GameObject* obj = node->getGameObject();
			MonoType* _type = mono_reflection_type_get_type(type);
			MonoClass* _class = mono_type_get_class(_type);
			std::string _className = mono_class_get_name(_class);
			_className = CP_SYS(_className);

			Component* component = obj->getComponent(_className);
			if (component != nullptr)
				return component->getManagedObject();
		}

		return nullptr;
	}

	void API_MonoBehaviour::destroyGameObject(MonoObject * this_ptr, MonoObject * gameObject)
	{
		GameObject* node = nullptr;
		mono_field_get_value(gameObject, APIManager::getSingleton()->gameobject_ptr_field, reinterpret_cast<void*>(&node));

		if (node != nullptr)
			APIManager::getSingleton()->addDestroyObject(node);
	}

	void API_MonoBehaviour::destroyComponent(MonoObject * this_ptr, MonoObject * component)
	{	
		Component* comp = nullptr;
		mono_field_get_value(component, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&comp));

		if (comp != nullptr)
		{
			GameObject* obj = comp->getGameObject();
			obj->removeComponent(comp);
		}
	}

	MonoObject* searchTypeRecursive(MonoClass* _class, bool findInactive)
	{
		MonoObject* outType = nullptr;

		std::vector<GameObject*> objects = Engine::getSingleton()->getGameObjects();
		for (auto it = objects.begin(); it != objects.end(); ++it)
		{
			GameObject* curNode = *it;

			if (outType != nullptr)
				break;

			if (!findInactive)
			{
				if (!curNode->getActive())
					continue;
			}

			for (auto _it = curNode->getComponents().begin(); _it != curNode->getComponents().end(); ++_it)
			{
				Component* comp = *_it;

				if (!findInactive)
				{
					if (!comp->getEnabled())
						continue;
				}

				if (comp->getManagedObject() == nullptr)
					continue;

				MonoClass* cls = mono_object_get_class(comp->getManagedObject());
				bool isSubClass = mono_class_is_subclass_of(cls, _class, true);

				if (cls == _class || isSubClass)
				{
					outType = comp->getManagedObject();
					break;
				}
			}
		}

		return outType;
	}

	void searchTypesRecursive(MonoClass* _class, std::vector<MonoObject*>& outList, bool findInactive)
	{
		std::vector<GameObject*> objects = Engine::getSingleton()->getGameObjects();
		for (auto it = objects.begin(); it != objects.end(); ++it)
		{
			GameObject* curNode = *it;

			if (!findInactive)
			{
				if (!curNode->getActive())
					continue;
			}

			for (auto _it = curNode->getComponents().begin(); _it != curNode->getComponents().end(); ++_it)
			{
				Component* comp = *_it;
				if (!findInactive)
				{
					if (!comp->getEnabled())
						continue;
				}

				if (comp->getManagedObject() == nullptr)
					continue;

				MonoClass* cls = mono_object_get_class(comp->getManagedObject());
				bool isSubClass = mono_class_is_subclass_of(cls, _class, true);

				if (cls == _class || isSubClass)
				{
					outList.push_back(comp->getManagedObject());
				}
			}
		}
	}

	MonoObject* API_MonoBehaviour::findObjectOfTypeT(MonoReflectionType* type, bool findInactive)
	{
		MonoType* _type = mono_reflection_type_get_type(type);
		MonoClass* _class = mono_type_get_class(_type);

		MonoObject* obj = searchTypeRecursive(_class, findInactive);

		return obj;
	}

	MonoArray* API_MonoBehaviour::findObjectsOfTypeT(MonoReflectionType* type, bool findInactive)
	{
		MonoType* _type = mono_reflection_type_get_type(type);
		MonoClass* _class = mono_type_get_class(_type);

		std::vector<MonoObject*> objs;
		searchTypesRecursive(_class, objs, findInactive);

		MonoArray* arr = mono_array_new(APIManager::getSingleton()->getDomain(), _class, objs.size());

		int i = 0;
		for (auto it = objs.begin(); it != objs.end(); ++it, ++i)
		{
			mono_array_setref(arr, i, *it);
		}

		return arr;
	}
}