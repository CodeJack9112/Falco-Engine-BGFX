#include "API_SceneManager.h"

#include "../Core/Engine.h"
#include "../Core/APIManager.h"
#include "../Assets/Scene.h"
#include "../Classes/IO.h"
#include "../Classes/StringConverter.h"

namespace GX
{
	MonoString * API_SceneManager::getLoadedScene()
	{
		std::string loadedScene = Scene::getLoadedScene();

		MonoString * path = mono_string_new(APIManager::getSingleton()->getDomain(), CP_UNI(loadedScene).c_str());

		return path;
	}

	void API_SceneManager::loadScene(MonoObject * path)
	{
		std::string _path = (const char*)mono_string_to_utf8((MonoString*)path);
		_path = CP_SYS(_path);

		APIManager::getSingleton()->sceneToLoad = _path;
	}
}