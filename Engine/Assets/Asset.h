#pragma once

#include <map>
#include <string>

#include "../../Mono/include/mono/metadata/object.h"

namespace GX
{
	class Asset
	{
	protected:
		bool loaded = false;
		bool persistent = false;
		static std::map<std::string, Asset*> loadedInstances;

		std::string alias = "";
		std::string name = "";
		std::string location = "";

		MonoClass* managedClass = nullptr;
		MonoObject* managedObject = nullptr;
		uint32_t managedGCHandle = 0;

	public:
		Asset(MonoClass* monoClass);
		virtual ~Asset();

		virtual void load();
		virtual void unload();
		virtual std::string getAssetType() { return "Asset"; }
		virtual void reload();

		bool isLoaded() { return loaded; }

		//void createManagedObject(MonoObject* obj);
		void createManagedObject();
		void destroyManagedObject();

		static Asset* getLoadedInstance(std::string location, std::string name);
		static void setLoadedInstance(std::string location, std::string name, Asset* instance);
		static std::map<std::string, Asset*>& getLoadedInstances() { return loadedInstances; }
		static void unloadAll();

		std::string getName() { return name; }
		void setName(std::string value) { name = value; }

		std::string getAlias() { return alias; }
		void setAlias(std::string value) { alias = value; }

		std::string getLocation() { return location; }
		void setLocation(std::string value) { location = value; }

		std::string getOrigin() { return location + name; }

		void setPersistent(bool value) { persistent = value; }
		bool getPersistent() { return persistent; }

		MonoObject* getManagedObject() { return managedObject; }
		MonoClass* getManagedClass() { return managedClass; }
		uint32_t getManagedGCHandle() { return managedGCHandle; }

		void setManagedObject(MonoObject* value);
	};
}