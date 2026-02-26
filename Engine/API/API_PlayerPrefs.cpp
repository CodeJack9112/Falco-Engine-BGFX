#include "API_PlayerPrefs.h"

#include "../Core/Engine.h"
#include "../Core/APIManager.h"
#include "../Serialization/Settings/PlayerPrefs.h"
#include "../Classes/StringConverter.h"
#include "../Classes/IO.h"

#include <string>

namespace GX
{
	bool API_PlayerPrefs::trySetInt(MonoString* key, int value)
	{
		std::string keyStr = (const char*)mono_string_to_utf8(key);
		keyStr = CP_SYS(keyStr);
		
		PlayerPrefs::Singleton.setInt(keyStr, value);

		return true;
	}

	bool API_PlayerPrefs::trySetFloat(MonoString* key, float value)
	{
		std::string keyStr = (const char*)mono_string_to_utf8(key);
		keyStr = CP_SYS(keyStr);

		PlayerPrefs::Singleton.setFloat(keyStr, value);

		return true;
	}

	bool API_PlayerPrefs::trySetString(MonoString* key, MonoString* value)
	{
		std::string keyStr = (const char*)mono_string_to_utf8(key);
		keyStr = CP_SYS(keyStr);

		std::string valStr = (const char*)mono_string_to_utf8(value);
		valStr = CP_SYS(valStr);

		PlayerPrefs::Singleton.setString(keyStr, valStr);

		return true;
	}

	int API_PlayerPrefs::getInt(MonoString* key, int defaultValue)
	{
		std::string keyStr = (const char*)mono_string_to_utf8(key);
		keyStr = CP_SYS(keyStr);

		return PlayerPrefs::Singleton.getInt(keyStr, defaultValue);
	}

	float API_PlayerPrefs::getFloat(MonoString* key, float defaultValue)
	{
		std::string keyStr = (const char*)mono_string_to_utf8(key);
		keyStr = CP_SYS(keyStr);

		return PlayerPrefs::Singleton.getFloat(keyStr, defaultValue);
	}

	MonoString* API_PlayerPrefs::getString(MonoString* key, MonoString* defaultValue)
	{
		std::string keyStr = (const char*)mono_string_to_utf8(key);
		keyStr = CP_SYS(keyStr);

		std::string valStr = (const char*)mono_string_to_utf8(defaultValue);
		valStr = CP_SYS(valStr);

		std::string str = PlayerPrefs::Singleton.getString(keyStr, valStr);
		return mono_string_new(APIManager::getSingleton()->getDomain(), str.c_str());
	}

	bool API_PlayerPrefs::hasKey(MonoString* key)
	{
		std::string keyStr = (const char*)mono_string_to_utf8(key);
		keyStr = CP_SYS(keyStr);

		return PlayerPrefs::Singleton.hasKey(keyStr);
	}

	void API_PlayerPrefs::deleteKey(MonoString* key)
	{
		std::string keyStr = (const char*)mono_string_to_utf8(key);
		keyStr = CP_SYS(keyStr);

		PlayerPrefs::Singleton.deleteKey(keyStr);
	}

	void API_PlayerPrefs::deleteAll()
	{
		PlayerPrefs::Singleton.deleteAll();
	}

	void API_PlayerPrefs::save()
	{
		std::string libPath = Engine::getSingleton()->getLibraryPath();

		if (!IO::DirExists(libPath))
			libPath = Engine::getSingleton()->getRootPath();

		PlayerPrefs::Singleton.save(libPath + "PlayerPrefs.bin");
	}
}