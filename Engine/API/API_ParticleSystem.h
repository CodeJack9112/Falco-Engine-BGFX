#pragma once

#include "API.h"

namespace GX
{
	class API_ParticleSystem
	{
	public:
		//Register methods
		static void Register()
		{
			mono_add_internal_call("FalcoEngine.ParticleSystem::get_emittersCount", (void*)getEmittersCount);
			mono_add_internal_call("FalcoEngine.ParticleSystem::GetEmitter", (void*)getEmitter);

			mono_add_internal_call("FalcoEngine.ParticleEmitter::Play", (void*)emitterPlay);
			mono_add_internal_call("FalcoEngine.ParticleEmitter::Stop", (void*)emitterStop);
			mono_add_internal_call("FalcoEngine.ParticleEmitter::GetIsPlaying", (void*)emitterGetIsPlaying);
			mono_add_internal_call("FalcoEngine.ParticleEmitter::GetPlaybackTime", (void*)emitterGetPlaybackTime);
		}

	private:
		static int getEmittersCount(MonoObject* this_ptr);

		static MonoObject* getEmitter(MonoObject* this_ptr, int index);

		static void emitterPlay(MonoObject* this_ptr);

		static void emitterStop(MonoObject* this_ptr);

		static bool emitterGetIsPlaying(MonoObject* this_ptr);

		static float emitterGetPlaybackTime(MonoObject* this_ptr);
	};
}