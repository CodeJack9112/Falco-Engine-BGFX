#pragma once

#include "API.h"

namespace GX
{
	class API_GameObject
	{
	public:
		//Register methods
		static void Register()
		{
			mono_add_internal_call("FalcoEngine.GameObject::.ctor", (void*)ctor);
			mono_add_internal_call("FalcoEngine.GameObject::get_name", (void*)getName);
			mono_add_internal_call("FalcoEngine.GameObject::get_uniqueName", (void*)getUniqueName);
			mono_add_internal_call("FalcoEngine.GameObject::get_transform", (void*)getTransform);
			mono_add_internal_call("FalcoEngine.GameObject::get_rigidbody", (void*)getRigidbody);
			mono_add_internal_call("FalcoEngine.GameObject::get_animation", (void*)getAnimation);
			mono_add_internal_call("FalcoEngine.GameObject::get_audioSource", (void*)getAudioSource);
			mono_add_internal_call("FalcoEngine.GameObject::get_navMeshAgent", (void*)getNavMeshAgent);
			mono_add_internal_call("FalcoEngine.GameObject::get_components", (void*)getComponents);
			mono_add_internal_call("FalcoEngine.GameObject::get_enabled", (void*)getEnabled);
			mono_add_internal_call("FalcoEngine.GameObject::set_enabled", (void*)setEnabled);
			mono_add_internal_call("FalcoEngine.GameObject::get_active", (void*)getActive);
			mono_add_internal_call("FalcoEngine.GameObject::get_tag", (void*)getTag);
			mono_add_internal_call("FalcoEngine.GameObject::set_tag", (void*)setTag);
			mono_add_internal_call("FalcoEngine.GameObject::get_layer", (void*)getLayer);
			mono_add_internal_call("FalcoEngine.GameObject::set_layer", (void*)setLayer);
			mono_add_internal_call("FalcoEngine.GameObject::INTERNAL_find", (void*)findGameObject);
			mono_add_internal_call("FalcoEngine.GameObject::INTERNAL_getByUniqueName", (void*)getGameObject);
			mono_add_internal_call("FalcoEngine.GameObject::INTERNAL_get_mono_behaviour", (void*)getMonoBehaviour);
			mono_add_internal_call("FalcoEngine.GameObject::INTERNAL_get_mono_behaviour_t", (void*)getMonoBehaviourT);
			mono_add_internal_call("FalcoEngine.GameObject::INTERNAL_get_component_t", (void*)getComponentT);
			mono_add_internal_call("FalcoEngine.GameObject::INTERNAL_add_component_t", (void*)addComponentT);
			mono_add_internal_call("FalcoEngine.GameObject::Clone", (void*)clone);
		}

	private:

		//Constructor
		static void ctor(MonoObject* this_ptr);

		//Get alias name
		static MonoString * getName(MonoObject * this_ptr);

		//Get unique name
		static MonoString * getUniqueName(MonoObject * this_ptr);

		//Get tag
		static MonoString* getTag(MonoObject* this_ptr);

		//Set tag
		static void setTag(MonoObject* this_ptr, MonoString * value);

		//Get layer
		static int getLayer(MonoObject* this_ptr);

		//Set layer
		static void setLayer(MonoObject* this_ptr, int value);

		//Get visible
		static bool getEnabled(MonoObject * this_ptr);

		//Set visible
		static void setEnabled(MonoObject * this_ptr, bool visible);

		//Get active
		static bool getActive(MonoObject* this_ptr);

		//Get transform
		static MonoObject * getTransform(MonoObject * this_ptr);

		//Get rigidbody
		static MonoObject * getRigidbody(MonoObject * this_ptr);

		//Get animation
		static MonoObject * getAnimation(MonoObject * this_ptr);

		//Get audio source
		static MonoObject * getAudioSource(MonoObject * this_ptr);

		//Get navmesh agent
		static MonoObject * getNavMeshAgent(MonoObject * this_ptr);

		//Get all components
		static MonoArray* getComponents(MonoObject* this_ptr);

		//Find game object by alias name
		static MonoObject * findGameObject(MonoObject * name);

		//Find game object by unique name
		static MonoObject * getGameObject(MonoObject* name);

		//Get mono behaviour by class name
		static MonoObject * getMonoBehaviour(MonoObject * this_ptr, MonoObject * className);

		//Get mono behaviour by class type
		static MonoObject * getMonoBehaviourT(MonoObject * this_ptr, MonoReflectionType * type);

		//Get component by class type
		static MonoObject * getComponentT(MonoObject * this_ptr, MonoReflectionType * type);

		//Add component by class type
		static MonoObject* addComponentT(MonoObject* this_ptr, MonoReflectionType* type);
		
		//Clone
		static MonoObject* clone(MonoObject* this_ptr);
	};
}