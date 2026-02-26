#pragma once
#include "API.h"

namespace GX
{
	class API_Application
	{
	public:
		//Register methods
		static void Register()
		{
			mono_add_internal_call("FalcoEngine.Application::INTERNAL_quit", (void*)quit);
			mono_add_internal_call("FalcoEngine.Application::get_assetsPath", (void*)getAssetsPath);
		}

	private:
		static void quit();

		static MonoString * getAssetsPath();
	};
}