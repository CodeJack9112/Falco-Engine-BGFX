#include "GameObject.h"

#include <unordered_map>
#include <algorithm>

#include "Engine.h"
#include "Core/APIManager.h"
#include "Classes/GUIDGenerator.h"
#include "Components/Transform.h"
#include "Components/MeshRenderer.h"
#include "Components/MonoScript.h"
#include "Components/RigidBody.h"

#include "Classes/Hash.h"

namespace GX
{
	GameObject::GameObject()
	{
		setGuid(GUIDGenerator::genGuid());

		createManagedObject();

		transform = new Transform();
		addComponent(transform);
	}

	GameObject::GameObject(std::string _guid)
	{
		setGuid(_guid);

		createManagedObject();

		transform = new Transform();
		addComponent(transform);
	}

	GameObject::~GameObject()
	{
		//Delete components
		while (components.size() > 0)
		{
			Component* comp = *components.begin();

			if (comp->isAttached())
				comp->onDetach();

			components.erase(components.begin());

			if (comp != transform)
				delete comp;
		}

		while (transform->children.size() > 0)
		{
			Transform* t = *transform->children.begin();
			transform->children.erase(transform->children.begin());
			delete t->gameObject;
		}

		delete transform;
		transform = nullptr;

		if (destroyCallback != nullptr)
			destroyCallback();
	}

	void GameObject::createManagedObject()
	{
		if (!APIManager::getSingleton()->isInitialized())
			return;

		managedObject = mono_object_new(APIManager::getSingleton()->getDomain(), APIManager::getSingleton()->gameobject_class);

		//mono_runtime_object_init(managedObject);
		managedGCHandle = mono_gchandle_new(managedObject, true); // Prevent this object to be destroyed by garbage collector

		GameObject* _this = this;
		mono_field_set_value(managedObject, APIManager::getSingleton()->gameobject_ptr_field, reinterpret_cast<void*>(&_this));
	}

	void GameObject::destroyManagedObject()
	{
		if (!APIManager::getSingleton()->isInitialized())
			return;

		mono_field_set_value(managedObject, APIManager::getSingleton()->gameobject_ptr_field, nullptr);

		if (managedGCHandle > 0)
		{
			mono_gchandle_free(managedGCHandle);
			managedGCHandle = 0;
		}
	}

	void GameObject::setManagedObject(MonoObject* value)
	{
		destroyManagedObject();

		managedObject = value;
		managedGCHandle = mono_gchandle_new(managedObject, true); // Prevent this object to be destroyed by garbage collector

		GameObject* _this = this;
		mono_field_set_value(managedObject, APIManager::getSingleton()->gameobject_ptr_field, reinterpret_cast<void*>(&_this));
	}

	void GameObject::setName(std::string value)
	{
		name = value;
		nameHash = Hash::getHash(name);
	}

	void GameObject::setGuid(std::string value)
	{
		guid = value;
		guidHash = Hash::getHash(guid);
	}

	Component* GameObject::getComponent(std::string type)
	{
		auto it = std::find_if(components.begin(), components.end(), [=](Component* comp) { return comp->getComponentType() == type; });

		if (it != components.end())
			return *it;
		else
		{
			auto scripts = getMonoScripts();
			auto it = std::find_if(scripts.begin(), scripts.end(), [=](MonoScript* comp) { return comp->getClassName() == type; });

			if (it != scripts.end())
				return *it;
		}

		return nullptr;
	}

	Component* GameObject::getComponent(int index)
	{
		if (index < components.size())
			return components.at(index);

		return nullptr;
	}

	std::vector<MonoScript*> GameObject::getMonoScripts()
	{
		std::vector<MonoScript*> lst;
		for (auto it = components.begin(); it != components.end(); ++it)
		{
			if ((*it)->getComponentType() == MonoScript::COMPONENT_TYPE)
				lst.push_back((MonoScript*)*it);
		}

		return lst;
	}

	void GameObject::addComponent(Component* component)
	{
		assert(component->gameObject == nullptr && "This component is already attached to another object!");

		component->gameObject = this;
		components.push_back(component);

		if (component->getComponentType() == RigidBody::COMPONENT_TYPE)
			rigidBody = (RigidBody*)component;

		component->onAttach();
		component->onChangeParent(nullptr);
	}

	void GameObject::removeComponent(int index)
	{
		if (index < components.size())
		{
			Component* comp = *(components.begin() + index);
			if (comp->getComponentType() == Transform::COMPONENT_TYPE)
				return;

			if (rigidBody == comp)
				rigidBody = nullptr;

			components.erase(components.begin() + index);

			if (comp->isAttached())
				comp->onDetach();

			delete comp;
		}
	}

	void GameObject::removeComponent(Component* component)
	{
		if (component != nullptr)
		{
			if (component->getComponentType() == Transform::COMPONENT_TYPE)
				return;

			if (rigidBody == component)
				rigidBody = nullptr;

			auto it = std::find(components.begin(), components.end(), component);

			if (it != components.end())
				components.erase(it);

			if (component->isAttached())
				component->onDetach();

			delete component;
		}
	}

	void GameObject::removeComponent(std::string type)
	{
		Component* comp = getComponent(type);
		removeComponent(comp);
	}

	GameObject* GameObject::clone()
	{
		GameObject * ret = nullptr;

		std::unordered_map<GameObject*, GameObject*> remapList; //old, new

		std::vector<std::pair<Transform*, Transform*>> nstack; //child, parent
		nstack.push_back(std::make_pair(this->getTransform(), transform->getParent()));

		while (nstack.size() > 0)
		{
			Transform* child = nstack.begin()->first;
			Transform* parent = nstack.begin()->second;
			nstack.erase(nstack.begin());

			//Clone object
			GameObject * obj = child->getGameObject();
			GameObject * newObj = Engine::getSingleton()->createGameObject();
			if (ret == nullptr)
				ret = newObj;

			newObj->name = obj->name;
			newObj->nameHash = obj->nameHash;
			newObj->layer = obj->layer;
			newObj->batchingStatic = obj->batchingStatic;
			newObj->lightingStatic = obj->lightingStatic;
			newObj->navigationStatic = obj->navigationStatic;
			newObj->occlusionStatic = obj->occlusionStatic;
			newObj->enabled = obj->enabled;
			newObj->active = obj->active;
			newObj->serializable = obj->serializable;
			newObj->getTransform()->setTransformMatrix(obj->getTransform()->getTransformMatrix());

			newObj->getTransform()->setParent(parent);
			remapList[obj] = newObj;

			for (auto it = obj->components.begin(); it != obj->components.end(); ++it)
			{
				Component* comp = *it;
				if (comp->getComponentType() == Transform::COMPONENT_TYPE)
					continue;

				Component* newComp = comp->onClone();
				if (newComp != nullptr)
					newObj->addComponent(newComp);
			}
			//

			int j = 0;
			for (auto it = child->children.begin(); it != child->children.end(); ++it, ++j)
			{
				Transform* ch = *it;
				nstack.insert(nstack.begin() + j, std::make_pair(ch, newObj->getTransform()));
			}
		}

		//Post processing
		for (auto it = remapList.begin(); it != remapList.end(); ++it)
		{
			GameObject* oldObj = it->first;
			GameObject* newObj = it->second;

			for (auto c = newObj->components.begin(); c != newObj->components.end(); ++c)
			{
				for (auto r = remapList.begin(); r != remapList.end(); ++r)
				{
					(*c)->onRebindObject(r->first->getGuid(), r->second->getGuid());
				}
			}
		}

		for (auto it = remapList.begin(); it != remapList.end(); ++it)
		{
			GameObject* newObj = it->second;

			for (auto c = newObj->components.begin(); c != newObj->components.end(); ++c)
				(*c)->onRefresh();
		}

		remapList.clear();

		if (Engine::getSingleton()->getIsRuntimeMode())
			APIManager::getSingleton()->execute(ret, "Start");

		return ret;
	}

	void GameObject::setEnabled(bool value)
	{
		bool prevActive = active;

		enabled = value;
		active = value;
		if (transform->getParent() != nullptr)
		{
			if (transform->getParent()->getGameObject()->getActive())
				active = value;
			else
				active = false;
		}

		if (prevActive != active)
		{
			for (auto it = components.begin(); it != components.end(); ++it)
				(*it)->onStateChanged();

			std::vector<Transform*> nstack;
			for (auto it = transform->children.begin(); it != transform->children.end(); ++it)
				nstack.push_back(*it);

			while (nstack.size() > 0)
			{
				Transform* child = *nstack.begin();
				nstack.erase(nstack.begin());

				//Update state
				if (child->getGameObject()->enabled)
				{
					child->getGameObject()->active = active;

					std::vector<Component*>& childComponents = child->getGameObject()->getComponents();
					for (auto it = childComponents.begin(); it != childComponents.end(); ++it)
						(*it)->onStateChanged();
					//

					int j = 0;
					for (auto it = child->children.begin(); it != child->children.end(); ++it, ++j)
					{
						Transform* ch = *it;
						nstack.insert(nstack.begin() + j, ch);
					}
				}
			}
		}
	}

	void GameObject::setLayer(int value, bool recursive)
	{
		layer = value;

		RigidBody* body = (RigidBody*)getComponent(RigidBody::COMPONENT_TYPE);
		if (body != nullptr)
			body->updateCollisionMask();

		if (recursive)
		{
			std::vector<Transform*> nstack;
			for (auto it = transform->children.begin(); it != transform->children.end(); ++it)
				nstack.push_back(*it);

			while (nstack.size() > 0)
			{
				Transform* child = *nstack.begin();
				nstack.erase(nstack.begin());

				//Update state
				child->gameObject->layer = value;

				body = (RigidBody*)child->gameObject->getComponent(RigidBody::COMPONENT_TYPE);
				if (body != nullptr)
					body->updateCollisionMask();
				//

				int j = 0;
				for (auto it = child->children.begin(); it != child->children.end(); ++it, ++j)
				{
					Transform* ch = *it;
					nstack.insert(nstack.begin() + j, ch);
				}
			}
		}
	}

	void GameObject::setSerializable(bool value)
	{
		if (serializable == value)
			return;

		serializable = value;

		if (!serializable)
		{
			destroyManagedObject();
			Engine::getSingleton()->removeGameObject(this);
		}
		else
		{
			createManagedObject();
			Engine::getSingleton()->addGameObject(this);
		}
	}

	void GameObject::setLightingStatic(bool value)
	{
		lightingStatic = value;

		MeshRenderer* renderer = (MeshRenderer*)getComponent(MeshRenderer::COMPONENT_TYPE);
		if (renderer == nullptr)
			return;

		renderer->reloadLightmaps();
	}

	void GameObject::setBatchingStatic(bool value)
	{
		batchingStatic = value;
	}
}