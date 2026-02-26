#pragma once

#include <SDL2/SDL.h>

namespace GX
{
	class Time
	{
	private:
		static float timeScale;
		static float timeSinceGameStart;
		static float timeSinceGameStartScaled;
		static float timeSinceLevelStart;
		static float timeSinceLevelStartScaled;
		static float deltaTime;
		static float refreshRate;
		static float refreshTimer;
		static int avgFramerate[60];
		static int frameRate;
		static uint32_t now;
		static uint32_t last;

	public:
		static float getTimeScale() { return timeScale; }
		static void setTimeScale(float value) { timeScale = value; }

		static float getDeltaTime() { return deltaTime; }
		static float getTimeSinceLevelStart() { return timeSinceLevelStart; }
		static float getTimeSinceLevelStartScaled() { return timeSinceLevelStartScaled; }
		static float getTimeSinceGameStart() { return timeSinceGameStart; }
		static float getTimeSinceGameStartScaled() { return timeSinceGameStartScaled; }
		static int getFramesPerSecond();

		static void updateTime();
		static void resetTimeSinceLevelStart();
	};
}