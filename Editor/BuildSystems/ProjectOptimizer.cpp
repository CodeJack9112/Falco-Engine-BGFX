#include "ProjectOptimizer.h"

#include <fstream>
#include <iostream>

#include "../Dialogs/DialogProgress.h"

#include "../Engine/Core/Engine.h"
#include "../Engine/Renderer/Renderer.h"
#include "../Engine/Assets/Asset.h"
#include "../Engine/Assets/Scene.h"
#include "../Engine/Assets/Texture.h"
#include "../Engine/Assets/Cubemap.h"
#include "../Engine/Assets/Material.h"
#include "../Engine/Assets/Mesh.h"
#include "../Engine/Assets/Model3DLoader.h"
#include "../Engine/Assets/Prefab.h"
#include "../Engine/Classes/IO.h"
#include "../Engine/Classes/md5.h"
#include "../Engine/Renderer/BatchedGeometry.h"
#include "../Engine/Renderer/CSGGeometry.h"
#include "../Engine/Components/Terrain.h"

#include "../Engine/Serialization/Scene/SScene.h"
#include "../Engine/Serialization/Assets/STerrainData.h"

namespace GX
{
	ProjectOptimizer::ProjectOptimizer()
	{
		progressDialog = new DialogProgress();
		progressDialog->setTwoProgressBars(true);
	}

	ProjectOptimizer::~ProjectOptimizer()
	{
		delete progressDialog;
	}

	void ProjectOptimizer::update()
	{
		
	}

	void ProjectOptimizer::addPath(std::string path, std::vector<std::string>& usedAssets)
	{
		if (std::find(usedAssets.begin(), usedAssets.end(), path) == usedAssets.end())
			usedAssets.push_back(path);
	}

	void ProjectOptimizer::processTexture(Texture* tex, std::vector<std::string>& usedAssets)
	{
		if (tex != nullptr && tex->isLoaded())
		{
			if (!tex->getPersistent())
			{
				addPath(tex->getCachedFileName(), usedAssets);
			}
		}
	}

	void ProjectOptimizer::processMaterial(Material* mat, std::vector<std::string>& usedAssets)
	{
		if (mat == nullptr)
			return;

		addPath(mat->getOrigin(), usedAssets);

		for (auto& uni : mat->getUniforms())
		{
			if (uni.getType() == UniformType::Sampler2D)
			{
				Texture* _tex = uni.getValue<Sampler2DDef>().second;
				processTexture(_tex, usedAssets);
			}

			if (uni.getType() == UniformType::SamplerCube)
			{
				Cubemap* cube = uni.getValue<SamplerCubeDef>().second;
				if (cube != nullptr && cube->isLoaded())
				{
					if (!cube->getPersistent())
					{
						addPath(cube->getOrigin(), usedAssets);

						for (int i = 0; i < 6; ++i)
							processTexture(cube->getTexture(i), usedAssets);
					}
				}
			}
		}
	}

	void ProjectOptimizer::processMaterial(SMaterial* mat, std::string location, std::vector<std::string>& usedAssets)
	{
		if (mat == nullptr)
			return;

		for (auto& uni : mat->uniforms)
		{
			if (static_cast<UniformType>(uni.uniformType) == UniformType::Sampler2D)
			{
				Texture* _tex = Texture::load(location, uni.sampler2DVal.textureName);
				processTexture(_tex, usedAssets);
			}

			if (static_cast<UniformType>(uni.uniformType) == UniformType::SamplerCube)
			{
				Cubemap* cube = Cubemap::load(location, uni.samplerCubeVal.cubemapName);
				if (cube != nullptr && cube->isLoaded())
				{
					if (!cube->getPersistent())
					{
						addPath(cube->getOrigin(), usedAssets);

						for (int i = 0; i < 6; ++i)
							processTexture(cube->getTexture(i), usedAssets);
					}
				}
			}
		}
	}

	void ProjectOptimizer::processScene(std::string path, std::vector<std::string>& usedAssets, std::string format)
	{
		std::string libraryPath = Engine::getSingleton()->getLibraryPath();
		std::string location = Engine::getSingleton()->getAssetsPath();

		std::string fullPath = location + path;

		SScene scene;

		std::ifstream ofs(fullPath, std::ios::binary);
		BinarySerializer s;
		s.deserialize(&ofs, &scene, format);
		ofs.close();

		Material* skyMtl = Material::load(location, scene.skyboxMaterial);
		if (skyMtl != nullptr)
			processMaterial(skyMtl, usedAssets);

		//Check static geometry
		std::string geomName = IO::GetFilePath(path) + IO::GetFileName(path) + "/Static Geometry/" + md5(IO::GetFileName(path)) + ".mesh";

		std::string libPath = Engine::getSingleton()->getLibraryPath();

		if (IO::FileExists(libPath + geomName))
			addPath(libPath + geomName, usedAssets);

		//Batched geometry
		BatchedGeometry geom;
		geom.loadFromFile(libPath, geomName);

		for (auto& batch : geom.getBatches())
		{
			if (batch->getLightingStatic())
			{
				Texture* lightmap = batch->getLightmap();
				processTexture(lightmap, usedAssets);
			}
		}

		geom.clear();

		//CSG geometry
		for (auto& obj : scene.gameObjects)
		{
			for (auto& comp : obj.csgModels)
			{
				std::string hash = obj.guid;
				std::string csgName = IO::GetFilePath(path) + IO::GetFileName(path) + "/CSG Geometry/" + md5(IO::GetFileName(path) + "_" + hash + "_csg") + ".mesh";

				if (IO::FileExists(libPath + csgName))
					addPath(libPath + csgName, usedAssets);

				CSGGeometry csgGeom;
				csgGeom.loadFromFile(libPath, csgName, nullptr);

				for (auto& mdl : csgGeom.getModels())
				{
					for (auto& subMesh : mdl->subMeshes)
					{
						Texture* lightmap = subMesh->getLightmap();
						processTexture(lightmap, usedAssets);
					}
				}

				csgGeom.clear();
			}
		}

		//Cache resources
		int iter = 0;
		for (auto it = scene.gameObjects.begin(); it != scene.gameObjects.end(); ++it, ++iter)
		{
			SGameObject& sObj = *it;

			int progressEvery = (float)scene.gameObjects.size() * (5.0f / 100.0f);
			if (progressEvery > 0)
			{
				int progress = 100.0f / (float)scene.gameObjects.size() * (float)iter;
				if (iter % progressEvery == 0)
				{
					progressDialog->setProgress((float)progress / 100.0f, 1);
					progressDialog->setStatusText("Processing object: " + it->name, 1);
				}
			}

			for (auto& comp : sObj.audioSources)
				addPath(location + comp.fileName, usedAssets);

			for (auto& comp : sObj.videoPlayers)
				addPath(location + comp.videoClip, usedAssets);

			for (auto& comp : sObj.meshRenderers)
			{
				if (IO::FileExists(location + comp.meshSourceFile))
					Model3DLoader::cache3DModel(location, comp.meshSourceFile, nullptr);
				else if (IO::FileExists(libraryPath + comp.meshSourceFile))
					Model3DLoader::cache3DModel(libraryPath, comp.meshSourceFile, nullptr);
				addPath(libraryPath + comp.mesh, usedAssets);
				for (auto& m : comp.materials)
				{
					Material* mat = Material::load(location, m);
					if (mat != nullptr) processMaterial(mat, usedAssets);
				}

				//Lightmaps
				if (sObj.lightingStatic)
				{
					for (int m = 0; m < comp.materials.size(); ++m)
					{
						std::string lightmapPath = IO::GetFilePath(path) + IO::GetFileName(path) + "/Lightmaps/" + sObj.guid + "_" + std::to_string(m) + ".jpg";
						if (IO::FileExists(location + lightmapPath))
						{
							Texture* lightmap = Texture::load(location, lightmapPath);
							processTexture(lightmap, usedAssets);
						}
					}
				}

				Renderer::getSingleton()->frame();
			}

			for (auto& comp : sObj.buttons)
			{
				Texture* tex = Texture::load(location, comp.texNormalPath);
				processTexture(tex, usedAssets);
				tex = Texture::load(location, comp.texActivePath);
				processTexture(tex, usedAssets);
				tex = Texture::load(location, comp.texDisabledPath);
				processTexture(tex, usedAssets);
				tex = Texture::load(location, comp.texHoverPath);
				processTexture(tex, usedAssets);
			}

			for (auto& comp : sObj.images)
			{
				Texture* tex = Texture::load(location, comp.texturePath);
				processTexture(tex, usedAssets);
			}

			for (auto& comp : sObj.particleSystems)
			{
				for (auto& em : comp.emitters)
				{
					Material* mat = Material::load(location, em.material);
					if (mat != nullptr) processMaterial(mat, usedAssets);
				}
			}

			for (auto& comp : sObj.waters)
			{
				processMaterial(&comp.material, location, usedAssets);
			}

			for (auto& comp : sObj.monoScripts)
			{
				for (auto& f : comp.fieldList)
				{
					if (f.fieldType == "FalcoEngine.Prefab")
					{
						if (f.objectVal != "[None]")
							processScene(f.objectVal, usedAssets, Prefab::ASSET_TYPE);
					}

					if (f.fieldType == "FalcoEngine.Material")
					{
						if (f.objectVal != "[None]")
						{
							Material* mat = Material::load(location, f.objectVal);
							if (mat != nullptr) processMaterial(mat, usedAssets);
						}
					}

					if (f.fieldType == "FalcoEngine.Texture")
					{
						if (f.objectVal != "[None]")
						{
							Texture* tex = Texture::load(location, f.objectVal);
							processTexture(tex, usedAssets);
						}
					}

					if (f.fieldType == "FalcoEngine.AnimationClip" ||
						f.fieldType == "FalcoEngine.AudioClip" ||
						f.fieldType == "FalcoEngine.Cubemap" ||
						f.fieldType == "FalcoEngine.Font" ||
						f.fieldType == "FalcoEngine.Shader")
					{
						if (f.objectVal != "[None]")
							addPath(location + f.objectVal, usedAssets);
					}
				}
			}

			for (auto& comp : sObj.terrains)
			{
				STerrainData terrain;

				std::ifstream ofs1(location + comp.filePath, std::ios::binary);
				BinarySerializer s;
				s.deserialize(&ofs1, &terrain, Terrain::COMPONENT_TYPE);
				ofs1.close();

				Material* mat = Material::load(location, terrain.material);
				if (mat != nullptr) processMaterial(mat, usedAssets);

				for (auto& tex : terrain.textureList)
				{
					Texture* _tex = Texture::load(location, tex.diffuseTexture);
					processTexture(_tex, usedAssets);
					_tex = Texture::load(location, tex.normalTexture);
					processTexture(_tex, usedAssets);
				}

				for (auto& grass : terrain.grassList)
				{
					Material* mat = Material::load(location, grass.name);
					if (mat != nullptr) processMaterial(mat, usedAssets);
				}

				for (auto& tree : terrain.treeList)
				{
					if (IO::GetFileExtension(tree.name) == "prefab")
						processScene(tree.name, usedAssets, Prefab::ASSET_TYPE);
					else
					{
						auto meshes = Model3DLoader::load3DModelMeshes(location, tree.name);
						for (auto& m : meshes)
						{
							addPath(m.mesh->getOrigin(), usedAssets);
							for (auto& mt : m.materials)
								processMaterial(mt, usedAssets);
						}
						meshes.clear();
					}
				}

				for (auto& mesh : terrain.detailMeshList)
				{
					if (IO::GetFileExtension(mesh.name) == "prefab")
						processScene(mesh.name, usedAssets, Prefab::ASSET_TYPE);
					else
					{
						auto meshes = Model3DLoader::load3DModelMeshes(location, mesh.name);
						for (auto& m : meshes)
						{
							addPath(m.mesh->getOrigin(), usedAssets);
							for (auto& mt : m.materials)
								processMaterial(mt, usedAssets);
						}
						meshes.clear();
					}
				}
			}

			for (auto& comp : sObj.csgBrushes)
			{
				for (auto& f : comp.faces)
				{
					if (!f.material.empty())
					{
						Material* mat = Material::load(location, f.material);
						processMaterial(mat, usedAssets);
					}
					else
					{
						processMaterial(CSGGeometry::getDefaultMaterial(), usedAssets);
					}
				}
			}
		}
	}

	std::vector<std::string> ProjectOptimizer::getUsedResources()
	{
		progressDialog->show();
		progressDialog->setTitle("Preparing resources");

		std::vector<Asset*> loadedAssets;
		std::vector<std::string> usedAssets;

		for (auto& asset : Asset::getLoadedInstances())
		{
			if (asset.second->isLoaded() && !asset.second->getPersistent())
				loadedAssets.push_back(asset.second);
		}

		ProjectSettings* settings = Engine::getSingleton()->getSettings();
		auto& scenes = settings->getScenes();

		int s = 0;
		for (auto& scenePath : scenes)
		{
			float progress1 = 100.0f / (float)scenes.size() * s;
			progressDialog->setProgress((float)progress1 / 100.0f, 0);
			progressDialog->setStatusText(scenePath, 0);

			processScene(scenePath, usedAssets, Scene::ASSET_TYPE);

			//Unload newly cached resources
			std::vector<Asset*> delAssets;
			for (auto& asset : Asset::getLoadedInstances())
			{
				if (asset.second->isLoaded() && !asset.second->getPersistent())
				{
					auto it = std::find(loadedAssets.begin(), loadedAssets.end(), asset.second);
					if (it == loadedAssets.end())
						delAssets.push_back(asset.second);
				}
			}

			for (auto& asset : delAssets)
				delete asset;

			delAssets.clear();

			++s;
		}

		loadedAssets.clear();

		progressDialog->hide();

		BatchedGeometry::getSingleton()->reloadLightmaps();
		CSGGeometry::getSingleton()->reloadLightmaps();

		return usedAssets;
	}
}