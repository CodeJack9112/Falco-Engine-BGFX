#include "ProjectSettings.h"

#include "Classes/IO.h"
#include "Classes/md5.h"

#include <boost/serialization/export.hpp>
#include <boost/serialization/serialization.hpp>

#include "../../Core/Engine.h"
#include "../../Core/PhysicsManager.h"
#include "../../Core/Debug.h"
#include "../../Renderer/Renderer.h"
#include "../../Components/Light.h"
#include "../../Assets/Asset.h"
#include "../../Assets/Texture.h"

#include <SDL2/SDL.h>

namespace GX
{
	ProjectSettings::ProjectSettings()
	{
		for (int i = 0; i < 32; ++i)
		{
			for (int j = 0; j < 32; ++j)
			{
				collisionMatrix[i][j] = true;
			}
		}
	}

	void ProjectSettings::save()
	{
		std::string path = Engine::getSingleton()->getSettingsPath() + "Engine.settings";

		std::ofstream ofs(path, std::ios::binary);
		BinarySerializer s;
		s.serialize(&ofs, this, "Project Settings");
		ofs.close();
	}

	void ProjectSettings::load()
	{
		std::string path = Engine::getSingleton()->getSettingsPath() + "Engine.settings";

		if (IO::FileExists(path))
		{
			//Deserialize
			try
			{
				std::ifstream ofs(path, std::ios::binary);
				BinarySerializer s;
				s.deserialize(&ofs, this, "Project Settings");
				ofs.close();
			}
			catch(const std::exception& e)
			{
				Debug::logError(std::string("Project settings load error: ") + e.what());
				std::cerr << "Project settings load error: " << e.what() << std::endl;
			}
		}
	}

	void updateLights()
	{
		std::vector<Light*>& lights = Renderer::getSingleton()->getLights();
		for (auto it = lights.begin(); it != lights.end(); ++it)
		{
			(*it)->updateShadowMaps();
		}
	}

	void ProjectSettings::setUseDynamicResolution(bool value)
	{
		useDynamicResolution = value;
		Renderer::getSingleton()->resetFrameBuffers();
	}

	void ProjectSettings::setAutoDynamicResolution(bool value)
	{
		autoDynamicResolution = value;
		Renderer::getSingleton()->resetFrameBuffers();
	}

	void ProjectSettings::setDynamicResolution(float value)
	{
		dynamicResolution = value;
		Renderer::getSingleton()->resetFrameBuffers();
	}

	void ProjectSettings::setDirectionalShadowResolution(int value)
	{
		directionalShadowResolution = value;
		updateLights();
	}

	void ProjectSettings::setSpotShadowResolution(int value)
	{
		spotShadowResolution = value;
		updateLights();
	}

	void ProjectSettings::setPointShadowResolution(int value)
	{
		pointShadowResolution = value;
		updateLights();
	}

	void ProjectSettings::setShadowCascadesCount(int value)
	{
		shadowCascadesCount = value;
		updateLights();
	}

	void ProjectSettings::setShadowSamplesCount(int value)
	{
		shadowSamplesCount = value;
		updateLights();
	}

	void ProjectSettings::setShadowCascadesBlending(bool value)
	{
		shadowCascadesBlending = value;
		updateLights();
	}

	void ProjectSettings::setVSync(bool value)
	{
		vsync = value;
		
		if (vsync)
			SDL_GL_SetSwapInterval(1);
		else
			SDL_GL_SetSwapInterval(0);

		int w = Renderer::getSingleton()->getWidth();
		int h = Renderer::getSingleton()->getHeight();

		Renderer::getSingleton()->setSize(w, h);
	}

	void reloadTextures(std::function<void(std::string status, int progress)> callback)
	{
		std::vector<std::string> texFiles;
		auto& exts = Engine::getImageFileFormats();

		IO::listFiles(Engine::getSingleton()->getAssetsPath(), true, nullptr, [=, &texFiles](std::string dir, std::string name) -> bool
			{
				std::string filePath = dir + name;
				std::string ext = IO::GetFileExtension(filePath);

				if (std::find(exts.begin(), exts.end(), ext) != exts.end())
					texFiles.push_back(filePath);

				return true;
			}
		);

		int count = texFiles.size();

		int i = 0;
		for (auto& texFile : texFiles)
		{
			int progress = 100.0f / count * i;
			std::string texName = IO::GetFileNameWithExt(texFile);
			std::string str = texName + " (" + std::to_string(i) + "/" + std::to_string(count) + ")";

			if (callback != nullptr)
				callback(str, progress);

			///

			Texture* texture = nullptr;
			for (auto& asset : Asset::getLoadedInstances())
			{
				if (asset.second->getAssetType() == Texture::ASSET_TYPE && asset.second->isLoaded())
				{
					if (asset.first == texFile)
					{
						texture = (Texture*)asset.second;
						break;
					}
				}
			}

			if (texture != nullptr)
			{
				std::string texCache = texture->getCachedFileName();
				if (IO::FileExists(texCache))
					IO::FileDelete(texCache);
				texture->reload();
			}
			else
			{
				std::string location = Engine::getSingleton()->getAssetsPath();
				std::string name = IO::RemovePart(texFile, location);

				std::string libLocation = Engine::getSingleton()->getLibraryPath();
				std::string texDir = IO::GetFilePath(name);
				std::string texName = libLocation + texDir + md5(name) + ".texture";

				if (IO::FileExists(texName))
				{
					texture = Texture::load(location, name);
					if (texture != nullptr)
					{
						IO::FileDelete(texName);
						texture->reload();
						delete texture;
					}
				}
			}

			++i;
		}
	}

	void ProjectSettings::setTextureCompression(int value, std::function<void(std::string status, int progress)> callback)
	{
		textureCompression = value;
		reloadTextures(callback);
	}

	void ProjectSettings::setTextureCompressionQuality(int value, std::function<void(std::string status, int progress)> callback)
	{
		textureCompressionQuality = value;
		reloadTextures(callback);
	}

	void ProjectSettings::setTextureMaxResolution(int value, std::function<void(std::string status, int progress)> callback)
	{
		textureMaxResolution = value;
		reloadTextures(callback);
	}

	void ProjectSettings::setCollisionMask(int i, int j, bool value)
	{
		collisionMatrix[i][j] = value;
		PhysicsManager::getSingleton()->updateCollisionMatrix();
	}
}