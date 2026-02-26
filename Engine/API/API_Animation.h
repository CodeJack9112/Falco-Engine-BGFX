#pragma once

#include "API.h"

namespace GX
{
	namespace API
	{
		struct AnimationClipInfo
		{
			MonoObject* clip;
			MonoString* name;
			float speed;
			int startFrame;
			int endFrame;
			bool loop;
		};
	}

	class API_Animation
	{
	public:
		//Register methods
		static void Register()
		{
			mono_add_internal_call("FalcoEngine.Animation::INTERNAL_play", (void*)play);
			mono_add_internal_call("FalcoEngine.Animation::INTERNAL_crossFade", (void*)crossFade);
			mono_add_internal_call("FalcoEngine.Animation::INTERNAL_stop", (void*)stop);
			mono_add_internal_call("FalcoEngine.Animation::INTERNAL_setLoop", (void*)setLoop);
			mono_add_internal_call("FalcoEngine.Animation::INTERNAL_getLoop", (void*)getLoop);
			mono_add_internal_call("FalcoEngine.Animation::INTERNAL_isPlaying", (void*)isPlayingByName);
			mono_add_internal_call("FalcoEngine.Animation::get_isPlaying", (void*)isPlaying);
			mono_add_internal_call("FalcoEngine.Animation::get_numAnimationClips", (void*)getNumAnimationClips);
			mono_add_internal_call("FalcoEngine.Animation::INTERNAL_getAnimationClipInfo", (void*)getAnimationClipInfo);
		}

	private:
		//Play
		static void play(MonoObject * this_ptr, MonoString * name);

		//Cross fade
		static void crossFade(MonoObject* this_ptr, MonoString* name, float duration);

		//Stop
		static void stop(MonoObject * this_ptr);

		//Set loop
		static void setLoop(MonoObject * this_ptr, MonoString * name, bool loop);

		//Get loop
		static bool getLoop(MonoObject * this_ptr, MonoString * name);

		//Is playing by name
		static bool isPlayingByName(MonoObject * this_ptr, MonoString * name);

		//Is playing global
		static bool isPlaying(MonoObject * this_ptr);

		//Get animation clips count
		static int getNumAnimationClips(MonoObject* this_ptr);

		//Get animation clip info
		static void getAnimationClipInfo(MonoObject* this_ptr, int index, API::AnimationClipInfo* out_info);
	};
}