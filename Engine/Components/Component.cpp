#include "Component.h"

#include "Transform.h"
#include "../Core/APIManager.h"

namespace GX
{
	Component::Component(MonoClass* monoClass)
	{
		if (monoClass != nullptr)
		{
			managedClass = monoClass;
			createManagedObject();
		}
	}

	Component::~Component()
	{
		destroyManagedObject();
	}

	void Component::setEnabled(bool value)
	{
		enabled = value;
		onStateChanged();
	}

	void Component::createManagedObject()
	{
		if (!APIManager::getSingleton()->isInitialized())
			return;

		if (managedClass == nullptr)
			return;

		managedObject = mono_object_new(APIManager::getSingleton()->getDomain(), managedClass);

		//mono_runtime_object_init(managedObject);
		APIManager::getSingleton()->execute(managedObject, ".ctor");
		managedGCHandle = mono_gchandle_new(managedObject, true); // Prevent this object to be destroyed by garbage collector

		Component* _this = this;
		mono_field_set_value(managedObject, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&_this));
	}

	void Component::destroyManagedObject()
	{
		if (!APIManager::getSingleton()->isInitialized())
			return;

		if (managedObject == nullptr)
			return;

		mono_field_set_value(managedObject, APIManager::getSingleton()->component_ptr_field, nullptr);

		if (managedGCHandle > 0)
		{
			mono_gchandle_free(managedGCHandle);
			managedGCHandle = 0;
		}

		managedObject = nullptr;

		onManagedObjectDestroyed();
	}
}