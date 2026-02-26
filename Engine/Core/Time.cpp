#include "Time.h"

#include <bx/bx.h>
#include <bx/timer.h>

#include <string.h>

namespace GX
{
	float Time::timeScale = 1.0f;
	float Time::timeSinceGameStart = 0.0f;
	float Time::timeSinceGameStartScaled = 0.0f;
	float Time::timeSinceLevelStart = 0.0f;
	float Time::timeSinceLevelStartScaled = 0.0f;
	float Time::deltaTime = 0.0f;
	float Time::refreshRate = 0.5f;
	float Time::refreshTimer = 0.0f;
	int Time::avgFramerate[60];
	int Time::frameRate = 0;
	uint32_t Time::now = 0;
	uint32_t Time::last = 0;

	int Time::getFramesPerSecond()
	{
		return frameRate;
	}

	void Time::updateTime()
	{
		//Update delta time
		last = now;
		now = SDL_GetTicks();

		deltaTime = (float)(now - last) / 1000.0f;

		//Update fps
		int fps = (int)(1.0f / deltaTime);

		for (int i = 59; i > 0; --i)
			avgFramerate[i] = avgFramerate[i - 1];

		avgFramerate[0] = fps;

		if (refreshTimer < refreshRate)
		{
			refreshTimer += deltaTime;
		}
		else
		{
			refreshTimer = 0.0f;

			int _frameRate = 0;
			for (int i = 0; i < 60; ++i)
				_frameRate += avgFramerate[i];

			_frameRate /= 60;

			frameRate = _frameRate;
		}

		//Update counters
		timeSinceGameStart += deltaTime;
		timeSinceGameStartScaled += deltaTime * timeScale;
		timeSinceLevelStart += deltaTime;
		timeSinceLevelStartScaled += deltaTime * timeScale;
	}

	void Time::resetTimeSinceLevelStart()
	{
		memset(avgFramerate, 0, 60 * sizeof(int));

		now = SDL_GetTicks();
		last = now;

		frameRate = 0.0f;
		deltaTime = 0.0f;
		timeSinceLevelStart = 0.0f;
		timeSinceLevelStartScaled = 0.0f;
	}
}