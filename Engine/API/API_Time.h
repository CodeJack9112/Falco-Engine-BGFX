#pragma once

#include "API.h"

namespace GX
{
	class API_Time
	{
	public:
		//Register methods
		static void Register()
		{
			mono_add_internal_call("FalcoEngine.Time::get_frameRate", (void*)getFrameRate);
			mono_add_internal_call("FalcoEngine.Time::get_deltaTime", (void*)getDeltaTime);
			mono_add_internal_call("FalcoEngine.Time::get_timeScale", (void*)getTimeScale);
			mono_add_internal_call("FalcoEngine.Time::set_timeScale", (void*)setTimeScale);

			mono_add_internal_call("FalcoEngine.Time::get_timeSinceGameStart", (void*)getTimeSinceGameStart);
			mono_add_internal_call("FalcoEngine.Time::get_timeSinceGameStartScaled", (void*)getTimeSinceGameStartScaled);
			mono_add_internal_call("FalcoEngine.Time::get_timeSinceLevelStart", (void*)getTimeSinceLevelStart);
			mono_add_internal_call("FalcoEngine.Time::get_timeSinceLevelStartScaled", (void*)getTimeSinceLevelStartScaled);
		}

	private:
		static int getFrameRate();
		static float getDeltaTime();
		static float getTimeScale();
		static void setTimeScale(float value);

		static float getTimeSinceGameStart();
		static float getTimeSinceGameStartScaled();
		static float getTimeSinceLevelStart();
		static float getTimeSinceLevelStartScaled();
	};
}