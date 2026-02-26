#pragma once

#include "../../Mono/include/mono/metadata/object.h"

#include <string>
#include <vector>
#include <functional>

namespace GX
{
	class Component;
	class MonoScript;
	class Transform;
	class MeshRenderer;
	class RigidBody;

	class GameObject
	{
		friend class APIManager;
		friend class Transform;

	private:
		std::vector<Component*> components;
		Transform* transform = nullptr;
		RigidBody* rigidBody = nullptr;
		std::string name = "";
		std::string guid = "";
		int tag = 0;
		int layer = 0;

		size_t nameHash = 0;
		size_t guidHash = 0;

		bool serializable = true;
		bool enabled = true;
		bool active = true;
		bool navigationStatic = false;
		bool lightingStatic = false;
		bool batchingStatic = false;
		bool occlusionStatic = false;

		MonoObject* managedObject = nullptr;
		uint32_t managedGCHandle = 0;

		void setGuid(std::string value);

		std::function<void()> destroyCallback = nullptr;

		void createManagedObject();
		void destroyManagedObject();

	public:
		GameObject();
		GameObject(std::string _guid);
		~GameObject();

		void setOnDestroyCallback(std::function<void()> value) { destroyCallback = value; }

		std::vector<Component*>& getComponents() { return components; }

		Component* getComponent(std::string type);
		Component* getComponent(int index);
		Transform* getTransform() { return transform; }
		RigidBody* getRigidBody() { return rigidBody; }
		std::vector<MonoScript*> getMonoScripts();

		MonoObject* getManagedObject() { return managedObject; }
		void setManagedObject(MonoObject* value);

		void addComponent(Component* component);
		void removeComponent(int index);
		void removeComponent(Component* component);
		void removeComponent(std::string type);

		GameObject* clone();

		std::string getName() { return name; }
		size_t getNameHash() { return nameHash; }
		void setName(std::string value);
		std::string getGuid() { return guid; }
		size_t getGuidHash() { return guidHash; }

		bool isSerializable() { return serializable; }
		void setSerializable(bool value);

		bool getEnabled() { return enabled; }
		void setEnabled(bool value);

		bool getActive() { return enabled && active; }

		int getTag() { return tag; }
		void setTag(int value) { tag = value; };

		int getLayer() { return layer; }
		void setLayer(int value, bool recursive = false);

		bool getNavigationStatic() { return navigationStatic; }
		void setNavigationStatic(bool value) { navigationStatic = value; }

		bool getLightingStatic() { return lightingStatic; }
		void setLightingStatic(bool value);

		bool getBatchingStatic() { return batchingStatic; }
		void setBatchingStatic(bool value);

		bool getOcclusionStatic() { return occlusionStatic; }
		void setOcclusionStatic(bool value) { occlusionStatic = value; }
	};
}