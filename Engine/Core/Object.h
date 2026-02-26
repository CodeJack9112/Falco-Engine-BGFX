#pragma once

#include "../../Mono/include/mono/metadata/object.h"

namespace GX
{
	class Object
	{
	private:
		MonoClass* managedClass = nullptr;
		MonoObject* managedObject = nullptr;
		uint32_t managedGCHandle = 0;

	public:
		Object(MonoClass* monoClass);
		virtual ~Object();

		MonoObject* getManagedObject() { return managedObject; }
		MonoClass* getManagedClass() { return managedClass; }
		uint32_t getManagedGCHandle() { return managedGCHandle; }

		//void createManagedObject(MonoObject* obj);
		void createManagedObject();
		void destroyManagedObject();

		void setManagedObject(MonoObject* value);
	};
}