#pragma once

#include <string>
#include <functional>
#include <vector>

#include "../glm/glm.hpp"

struct aiScene;

namespace GX
{
	class GameObject;
	class Mesh;
	class Material;

	class Model3DLoader
	{
	public:
		struct ModelMeshData
		{
		public:
			Mesh* mesh = nullptr;
			std::vector<Material*> materials;
			glm::mat4x4 transform = glm::identity<glm::mat4x4>();
		};

		Model3DLoader() {}
		~Model3DLoader() {}

		static GameObject* load3DModel(std::string location, std::string path);
		static std::vector<ModelMeshData> load3DModelMeshes(std::string location, std::string path);
		static std::vector<Model3DLoader::ModelMeshData> load3DModelMeshesFromPrefab(std::string location, std::string name);
		static void cache3DModel(std::string location, std::string path, std::function<void(float progress, std::string status)> progressCb = nullptr);

		static void extractAnimations(std::string location, std::string path);
		static void extractAnimations(const aiScene* scene, std::string location, std::string path);

		static void exportObjects(std::string path, std::vector<GameObject*>& objects, int format);
	};
}