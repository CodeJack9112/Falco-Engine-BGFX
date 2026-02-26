#pragma once

#include "API.h"

namespace GX
{
	class API_AudioSource
	{
	public:
		static void Register()
		{
			mono_add_internal_call("FalcoEngine.AudioSource::INTERNAL_play", (void*)play);
			mono_add_internal_call("FalcoEngine.AudioSource::INTERNAL_pause", (void*)pause);
			mono_add_internal_call("FalcoEngine.AudioSource::INTERNAL_resume", (void*)resume);
			mono_add_internal_call("FalcoEngine.AudioSource::INTERNAL_stop", (void*)stop);
			mono_add_internal_call("FalcoEngine.AudioSource::get_isPlaying", (void*)isPlaying);
			mono_add_internal_call("FalcoEngine.AudioSource::get_isPaused", (void*)isPaused);
			mono_add_internal_call("FalcoEngine.AudioSource::get_loop", (void*)getLoop);
			mono_add_internal_call("FalcoEngine.AudioSource::set_loop", (void*)setLoop);
			mono_add_internal_call("FalcoEngine.AudioSource::get_volume", (void*)getVolume);
			mono_add_internal_call("FalcoEngine.AudioSource::set_volume", (void*)setVolume);
			mono_add_internal_call("FalcoEngine.AudioSource::get_pitch", (void*)getPitch);
			mono_add_internal_call("FalcoEngine.AudioSource::set_pitch", (void*)setPitch);
			mono_add_internal_call("FalcoEngine.AudioSource::get_minDistance", (void*)getMinDistance);
			mono_add_internal_call("FalcoEngine.AudioSource::set_minDistance", (void*)setMinDistance);
			mono_add_internal_call("FalcoEngine.AudioSource::get_maxDistance", (void*)getMaxDistance);
			mono_add_internal_call("FalcoEngine.AudioSource::set_maxDistance", (void*)setMaxDistance);
			mono_add_internal_call("FalcoEngine.AudioSource::get_is2D", (void*)getIs2D);
			mono_add_internal_call("FalcoEngine.AudioSource::set_is2D", (void*)setIs2D);
			mono_add_internal_call("FalcoEngine.AudioSource::get_audioClip", (void*)getAudioClip);
			mono_add_internal_call("FalcoEngine.AudioSource::set_audioClip", (void*)setAudioClip);
			mono_add_internal_call("FalcoEngine.AudioSource::GetTotalLength", (void*)getTotalLength);
			mono_add_internal_call("FalcoEngine.AudioSource::GetPlaybackPosition", (void*)getPlaybackPosition);
			mono_add_internal_call("FalcoEngine.AudioSource::SetPlaybackPosition", (void*)setPlaybackPosition);
			mono_add_internal_call("FalcoEngine.AudioSource::get_playOnStart", (void*)getPlayOnStart);
			mono_add_internal_call("FalcoEngine.AudioSource::set_playOnStart", (void*)setPlayOnStart);
		}

		//Play
		static void play(MonoObject * this_ptr);

		//Pause
		static void pause(MonoObject* this_ptr);

		//Resume
		static void resume(MonoObject* this_ptr);

		//Stop
		static void stop(MonoObject * this_ptr);

		//Is playing
		static bool isPlaying(MonoObject * this_ptr);

		//Is paused
		static bool isPaused(MonoObject* this_ptr);

		//Get loop
		static bool getLoop(MonoObject * this_ptr);

		//Set loop
		static void setLoop(MonoObject * this_ptr, bool loop);

		//Get volume
		static float getVolume(MonoObject* this_ptr);

		//Set volume
		static void setVolume(MonoObject* this_ptr, float volume);

		//Get pitch
		static float getPitch(MonoObject* this_ptr);

		//Set pitch
		static void setPitch(MonoObject* this_ptr, float volume);

		//Get min distance
		static float getMinDistance(MonoObject* this_ptr);

		//Set min distance
		static void setMinDistance(MonoObject* this_ptr, float value);

		//Get max distance
		static float getMaxDistance(MonoObject* this_ptr);

		//Set max distance
		static void setMaxDistance(MonoObject* this_ptr, float value);

		//Get is 2D
		static bool getIs2D(MonoObject* this_ptr);

		//Set is 2D
		static void setIs2D(MonoObject* this_ptr, bool value);

		//Get audio clip
		static MonoObject * getAudioClip(MonoObject* this_ptr);

		//Set file name
		static void setAudioClip(MonoObject* this_ptr, MonoObject* value);

		//Get total length
		static int getTotalLength(MonoObject* this_ptr);

		//Get current position
		static int getPlaybackPosition(MonoObject* this_ptr);

		//Set current position
		static void setPlaybackPosition(MonoObject* this_ptr, int seconds);

		//Get play on start
		static bool getPlayOnStart(MonoObject* this_ptr);

		//Set play on start
		static void setPlayOnStart(MonoObject* this_ptr, bool value);
	};
}