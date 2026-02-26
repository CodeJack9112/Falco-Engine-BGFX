#pragma once

//API class for access all game objects in scene

#include "API.h"

namespace GX
{
	class API_MonoBehaviour
	{
	public:
		//Register methods
		static void Register()
		{
			mono_add_internal_call("FalcoEngine.MonoBehaviour::get_name", (void*)getName);
			mono_add_internal_call("FalcoEngine.MonoBehaviour::get_uniqueName", (void*)getUniqueName);
			mono_add_internal_call("FalcoEngine.MonoBehaviour::get_transform", (void*)getTransform);
			mono_add_internal_call("FalcoEngine.MonoBehaviour::get_rigidbody", (void*)getRigidbody);
			mono_add_internal_call("FalcoEngine.MonoBehaviour::get_gameObject", (void*)getGameObject);
			mono_add_internal_call("FalcoEngine.MonoBehaviour::get_animation", (void*)getAnimation);
			mono_add_internal_call("FalcoEngine.MonoBehaviour::get_audioSource", (void*)getAudioSource);
			mono_add_internal_call("FalcoEngine.MonoBehaviour::get_navMeshAgent", (void*)getNavMeshAgent);
			mono_add_internal_call("FalcoEngine.MonoBehaviour::get_components", (void*)getComponents);
			mono_add_internal_call("FalcoEngine.MonoBehaviour::INTERNAL_get_component_t", (void*)getComponentT);
			mono_add_internal_call("FalcoEngine.MonoBehaviour::INTERNAL_destroy_game_object", (void*)destroyGameObject);
			mono_add_internal_call("FalcoEngine.MonoBehaviour::INTERNAL_destroy_component", (void*)destroyComponent);
			mono_add_internal_call("FalcoEngine.MonoBehaviour::INTERNAL_find_object_of_type_t", (void*)findObjectOfTypeT);
			mono_add_internal_call("FalcoEngine.MonoBehaviour::INTERNAL_find_objects_of_type_t", (void*)findObjectsOfTypeT);
		}

	private:

		//Get alias name
		static MonoString * getName(MonoObject* this_ptr);

		//Get unique name
		static MonoString * getUniqueName(MonoObject* this_ptr);

		//Get transform
		static MonoObject * getTransform(MonoObject* this_ptr);

		//Get rigidbody
		static MonoObject * getRigidbody(MonoObject* this_ptr);

		//Get GameObject
		static MonoObject * getGameObject(MonoObject* this_ptr);

		//Get Animation
		static MonoObject * getAnimation(MonoObject* this_ptr);

		//Get audio source
		static MonoObject * getAudioSource(MonoObject* this_ptr);

		//Get navmesh agent
		static MonoObject * getNavMeshAgent(MonoObject* this_ptr);
		
		//Get all components
		static MonoArray * getComponents(MonoObject* this_ptr);

		//Get component by class type
		static MonoObject * getComponentT(MonoObject* this_ptr, MonoReflectionType* type);
		
		//Find object of type
		static MonoObject* findObjectOfTypeT(MonoReflectionType* type, bool findInactive);

		//Find objects of type
		static MonoArray* findObjectsOfTypeT(MonoReflectionType* type, bool findInactive);

		//Destroy game object
		static void destroyGameObject(MonoObject* this_ptr, MonoObject* gameObject);
		
		//Destroy component
		static void destroyComponent(MonoObject* this_ptr, MonoObject* component);
	};
}