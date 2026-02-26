#pragma once

#include <string>
#include <functional>

#include "../glm/vec3.hpp"
#include "../glm/gtc/quaternion.hpp"

namespace GX
{
	class SScene;
	class SGameObject;
	class GameObject;
	class Prefab;

	class Scene
	{
		friend class ProjectOptimizer;

	private:
		static void saveObject(GameObject* gameObject, SScene* scene);
		static void loadObject(SGameObject* sObj, GameObject* obj, std::function<void(float progress, std::string status)> progressCb = nullptr);
		static void load(std::string location, std::string name, SScene * scene, std::function<void(float progress, std::string status)> progressCb);

		static std::string loadedScene;

	public:
		Scene() = default;
		~Scene() {}

		static std::string ASSET_TYPE;

		static void save(std::string location, std::string name);
		static void load(std::string location, std::string name, std::function<void(float progress, std::string status)> progressCb = nullptr);

		static void savePrefab(std::string location, std::string name, GameObject* gameObject);
		static GameObject* loadPrefab(Prefab* prefab, glm::vec3 position, glm::quat rotation);

		static void clear();

		static std::string getLoadedScene() { return loadedScene; }
	};
}