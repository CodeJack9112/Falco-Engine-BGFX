#pragma once

#include <string>
#include <vector>
#include <map>

#include "../Serialization/Settings/ProjectSettings.h"

struct zip;
typedef struct zip zip_t;

namespace GX
{
	class GameObject;
	class Transform;

	class Engine
	{
		friend class Window;

	private:
		static Engine singleton;

		static std::vector<std::string> imageFileFormats;
		static std::vector<std::string> model3dFileFormats;
		static std::vector<std::string> audioFileFormats;
		static std::vector<std::string> videoFileFormats;

		std::string appName;
		std::string assetsPath;
		std::string rootPath;
		std::string assemblyPath;
		std::string builtinResourcesPath;
		std::string libraryPath;
		std::string settingsPath;

		std::vector<Transform*> rootTransforms;
		bool isRuntime = true;
		bool shouldExit = false;

		static ProjectSettings settings;

		std::map<std::string, zip_t*> zipArchives;
		void openArchive(std::string path);

		std::vector<GameObject*> gameObjectCache;
		bool needUpdateGameObjectCache = true;

	public:
		Engine();
		~Engine();

		static Engine* getSingleton() { return &singleton; }

		void clear();

		void loadPlugins();
		void unloadPlugins();

		ProjectSettings* getSettings() { return &settings; }

		void setIsRuntimeMode(bool value) { isRuntime = value; }
		bool getIsRuntimeMode() { return isRuntime; }

		bool getIsEditorMode();

		void markGameObjectsOutdated() { needUpdateGameObjectCache = true; }

		static std::vector<std::string>& getImageFileFormats() { return imageFileFormats; }
		static std::vector<std::string>& getModel3dFileFormats() { return model3dFileFormats; }
		static std::vector<std::string>& getAudioFileFormats() { return audioFileFormats; }
		static std::vector<std::string>& getVideoFileFormats() { return videoFileFormats; }

		void setAppName(std::string name) { appName = name; }
		void setAssetsPath(std::string path);
		void setRootPath(std::string path) { rootPath = path; }
		void setBuiltinResourcesPath(std::string path);
		void setAssemblyPath(std::string path) { assemblyPath = path; }
		void setLibraryPath(std::string path);
		void setSettingsPath(std::string path) { settingsPath = path; }

		std::string getAppName() { return appName; }
		std::string getAssetsPath() { return assetsPath; }
		std::string getAssemblyPath() { return assemblyPath; }
		std::string getBuiltinResourcesPath() { return builtinResourcesPath; }
		std::string getRootPath() { return rootPath; }
		std::string getLibraryPath() { return libraryPath; }
		std::string getSettingsPath() { return settingsPath; }

		zip_t* getZipArchive(std::string path);

		std::vector<GameObject*>& getGameObjects();
		std::vector<Transform*>& getRootTransforms();

		int getRootObjectIndex(GameObject* object);
		void setRootObjectIndex(GameObject* object, int index);

		void stopAllAnimations();
		void stopObjectAnimations(GameObject* object);

		GameObject* createGameObject();
		GameObject* createGameObject(std::string guid);

		void destroyGameObject(GameObject * gameObject);
		void addGameObject(GameObject * gameObject);
		void removeGameObject(GameObject * gameObject);

		GameObject* getGameObject(std::string guid);
		GameObject* getGameObject(size_t guidHash);
		GameObject* findGameObject(std::string name);
		GameObject* findGameObject(size_t nameHash);
		GameObject* findGameObject(std::string name, GameObject* root);
		GameObject* findGameObject(size_t nameHash, GameObject* root);
		int getGameObjectIndex(GameObject * obj);

		void quit() { shouldExit = true; }
	};
}