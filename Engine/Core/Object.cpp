#include "Object.h"

#include "../Core/APIManager.h"

namespace GX
{
	Object::Object(MonoClass* monoClass)
	{
		if (monoClass != nullptr)
		{
			managedClass = monoClass;
			createManagedObject();
		}
	}

	Object::~Object()
	{
		destroyManagedObject();
	}

	void Object::createManagedObject()
	{
		if (!APIManager::getSingleton()->isInitialized())
			return;

		if (managedClass == nullptr)
			return;

		managedObject = mono_object_new(APIManager::getSingleton()->getDomain(), managedClass);

		//mono_runtime_object_init(managedObject);
		//createManagedObject(managedObject);
		managedGCHandle = mono_gchandle_new(managedObject, true); // Prevent this object to be destroyed by garbage collector

		Object* _this = this;
		mono_field_set_value(managedObject, APIManager::getSingleton()->object_ptr_field, reinterpret_cast<void*>(&_this));
	}

	void Object::destroyManagedObject()
	{
		if (!APIManager::getSingleton()->isInitialized())
			return;

		if (managedObject == nullptr)
			return;

		mono_field_set_value(managedObject, APIManager::getSingleton()->object_ptr_field, nullptr);

		if (managedGCHandle > 0)
		{
			mono_gchandle_free(managedGCHandle);
			managedGCHandle = 0;
		}

		managedObject = nullptr;
	}

	void Object::setManagedObject(MonoObject* value)
	{
		destroyManagedObject();

		managedObject = value;
		managedGCHandle = mono_gchandle_new(managedObject, true); // Prevent this object to be destroyed by garbage collector

		Object* _this = this;
		mono_field_set_value(managedObject, APIManager::getSingleton()->object_ptr_field, reinterpret_cast<void*>(&_this));
	}
}