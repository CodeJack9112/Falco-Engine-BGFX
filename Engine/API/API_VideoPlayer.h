#pragma once

#include "API.h"

namespace GX
{
	class API_VideoPlayer
	{
	public:
		//Register methods
		static void Register()
		{
			mono_add_internal_call("FalcoEngine.VideoPlayer::Play", (void*)play);
			mono_add_internal_call("FalcoEngine.VideoPlayer::Pause", (void*)pause);
			mono_add_internal_call("FalcoEngine.VideoPlayer::Resume", (void*)resume);
			mono_add_internal_call("FalcoEngine.VideoPlayer::Stop", (void*)stop);
			mono_add_internal_call("FalcoEngine.VideoPlayer::get_isPlaying", (void*)isPlaying);
			mono_add_internal_call("FalcoEngine.VideoPlayer::get_isPaused", (void*)isPaused);
			mono_add_internal_call("FalcoEngine.VideoPlayer::get_loop", (void*)getLoop);
			mono_add_internal_call("FalcoEngine.VideoPlayer::set_loop", (void*)setLoop);
			mono_add_internal_call("FalcoEngine.VideoPlayer::get_videoClip", (void*)getVideoClip);
			mono_add_internal_call("FalcoEngine.VideoPlayer::set_videoClip", (void*)setVideoClip);
			mono_add_internal_call("FalcoEngine.VideoPlayer::GetTotalLength", (void*)getTotalLength);
			mono_add_internal_call("FalcoEngine.VideoPlayer::GetPlaybackPosition", (void*)getPlaybackPosition);
			mono_add_internal_call("FalcoEngine.VideoPlayer::SetPlaybackPosition", (void*)setPlaybackPosition);
			mono_add_internal_call("FalcoEngine.VideoPlayer::get_texture", (void*)getTexture);
			mono_add_internal_call("FalcoEngine.VideoPlayer::get_playOnStart", (void*)getPlayOnStart);
			mono_add_internal_call("FalcoEngine.VideoPlayer::set_playOnStart", (void*)setPlayOnStart);
		}

		//Play
		static void play(MonoObject* this_ptr);

		//Pause
		static void pause(MonoObject* this_ptr);

		//Resume
		static void resume(MonoObject* this_ptr);

		//Stop
		static void stop(MonoObject* this_ptr);

		//Is playing
		static bool isPlaying(MonoObject* this_ptr);

		//Is paused
		static bool isPaused(MonoObject* this_ptr);

		//Get loop
		static bool getLoop(MonoObject* this_ptr);

		//Set loop
		static void setLoop(MonoObject* this_ptr, bool loop);

		//Get audio clip
		static MonoObject* getVideoClip(MonoObject* this_ptr);

		//Set file name
		static void setVideoClip(MonoObject* this_ptr, MonoObject* value);

		//Get total length
		static int getTotalLength(MonoObject* this_ptr);

		//Get current position
		static int getPlaybackPosition(MonoObject* this_ptr);

		//Set current position
		static void setPlaybackPosition(MonoObject* this_ptr, int seconds);

		//Get texture
		static MonoObject* getTexture(MonoObject* this_ptr);

		//Get play on start
		static bool getPlayOnStart(MonoObject* this_ptr);

		//Set play on start
		static void setPlayOnStart(MonoObject* this_ptr, bool value);
	};
}