#include "API_Application.h"

#include "../Core/Engine.h"
#include "../Core/APIManager.h"
#include "../Classes/StringConverter.h"

namespace GX
{
	void API_Application::quit()
	{
		Engine::getSingleton()->quit();
	}

	MonoString* API_Application::getAssetsPath()
	{
		std::string _str = CP_UNI(Engine::getSingleton()->getAssetsPath());
		MonoString* str = mono_string_new(APIManager::getSingleton()->getDomain(), _str.c_str());

		return str;
	}
}