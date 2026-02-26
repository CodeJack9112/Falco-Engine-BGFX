#pragma once

#include "API.h"

namespace GX
{
	class API_AudioListener
	{
	public:
		static void Register()
		{
			mono_add_internal_call("FalcoEngine.AudioListener::get_volume", (void*)getVolume);
			mono_add_internal_call("FalcoEngine.AudioListener::set_volume", (void*)setVolume);

			mono_add_internal_call("FalcoEngine.AudioListener::get_paused", (void*)getPaused);
			mono_add_internal_call("FalcoEngine.AudioListener::set_paused", (void*)setPaused);
		}

		//Get volume
		static float getVolume(MonoObject* this_ptr);

		//Set volume
		static void setVolume(MonoObject* this_ptr, float volume);

		//Get paused
		static bool getPaused(MonoObject* this_ptr);

		//Set paused
		static void setPaused(MonoObject* this_ptr, bool value);
	};
}