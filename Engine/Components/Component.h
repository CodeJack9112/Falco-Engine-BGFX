#pragma once

#include <string>
#include <map>
#include <functional>

#include "../../Mono/include/mono/metadata/object.h"

namespace GX
{
	class GameObject;
	class Camera;
	class Transform;

	class Component
	{
		friend class GameObject;
		friend class APIManager;
		friend class API_GameObject;
		friend class API_MonoBehaviour;

	protected:
		GameObject* gameObject = nullptr;
		
		MonoClass* managedClass = nullptr;
		MonoObject* managedObject = nullptr;
		uint32_t managedGCHandle = 0;

		bool enabled = true;
		bool attached = false;

		void createManagedObject();
		void destroyManagedObject();

	public:
		Component(MonoClass* monoClass);
		virtual ~Component();

		virtual std::string getComponentType() { return "Component"; }

		virtual void onAttach() { attached = true; }
		virtual void onDetach() { attached = false; }
		bool isAttached() { return attached; }
		virtual void onSceneLoaded() {}
		virtual void onUpdate(float deltaTime) {}
		virtual void onStateChanged() {}
		virtual Component* onClone() { return nullptr; }
		virtual void onRebindObject(std::string oldObj, std::string newObj) {}
		virtual void onRefresh() {}
		virtual void setEnabled(bool value);
		virtual void onManagedObjectDestroyed() {}
		virtual void onChangeParent(Transform* prevParent) {}
		virtual bool isEqualsTo(Component* other) { return true; }
		virtual bool isUiComponent() { return false; }
		virtual void onScreenResized(int w, int h) {}
		bool getEnabled() { return enabled; }

		MonoObject* getManagedObject() { return managedObject; }
		MonoClass* getManagedClass() { return managedClass; }
		uint32_t getManagedGCHandle() { return managedGCHandle; }

		GameObject* getGameObject() { return gameObject; }
	};
}