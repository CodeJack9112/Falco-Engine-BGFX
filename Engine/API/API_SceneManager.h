#pragma once

#include "API.h"

namespace GX
{
	class API_SceneManager
	{
	public:
		//Register methods
		static void Register()
		{
			mono_add_internal_call("FalcoEngine.SceneManager::get_loadedScene", (void*)getLoadedScene);
			mono_add_internal_call("FalcoEngine.SceneManager::INTERNAL_load_scene", (void*)loadScene);
		}

	private:
		static MonoString * getLoadedScene();
		static void loadScene(MonoObject * path);
	};
}