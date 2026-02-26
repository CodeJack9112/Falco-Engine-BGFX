#pragma once

#include "API.h"

namespace GX
{
	class API_PlayerPrefs
	{
	public:
		static void Register()
		{
			mono_add_internal_call("FalcoEngine.PlayerPrefs::TrySetInt", (void*)trySetInt);
			mono_add_internal_call("FalcoEngine.PlayerPrefs::TrySetFloat", (void*)trySetFloat);
			mono_add_internal_call("FalcoEngine.PlayerPrefs::TrySetString", (void*)trySetString);
			mono_add_internal_call("FalcoEngine.PlayerPrefs::GetInt", (void*)getInt);
			mono_add_internal_call("FalcoEngine.PlayerPrefs::GetFloat", (void*)getFloat);
			mono_add_internal_call("FalcoEngine.PlayerPrefs::GetString", (void*)getString);
			mono_add_internal_call("FalcoEngine.PlayerPrefs::HasKey", (void*)hasKey);
			mono_add_internal_call("FalcoEngine.PlayerPrefs::DeleteKey", (void*)deleteKey);
			mono_add_internal_call("FalcoEngine.PlayerPrefs::DeleteAll", (void*)deleteAll);
			mono_add_internal_call("FalcoEngine.PlayerPrefs::Save", (void*)save);
		}

		//Set int
		static bool trySetInt(MonoString * key, int value);

		//Set float
		static bool trySetFloat(MonoString * key, float value);

		//Set string
		static bool trySetString(MonoString * key, MonoString * value);

		//Get int
		static int getInt(MonoString * key, int defaultValue);

		//Get float
		static float getFloat(MonoString * key, float defaultValue);

		//Get string
		static MonoString * getString(MonoString * key, MonoString * defaultValue);

		//Has key
		static bool hasKey(MonoString * key);

		//Delete key
		static void deleteKey(MonoString * key);

		//Delete all
		static void deleteAll();

		//Save prefs
		static void save();
	};
}