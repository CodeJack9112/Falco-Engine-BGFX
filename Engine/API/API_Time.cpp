#include "API_Time.h"

#include "../Core/Time.h"
#include "../Renderer/Renderer.h"

namespace GX
{
	int API_Time::getFrameRate()
	{
		return Time::getFramesPerSecond();
	}

	float API_Time::getDeltaTime()
	{
		return Time::getDeltaTime();
	}

	float API_Time::getTimeScale()
	{
		return Time::getTimeScale();
	}

	void API_Time::setTimeScale(float value)
	{
		Time::setTimeScale(value);
	}

	float API_Time::getTimeSinceGameStart()
	{
		return Time::getTimeSinceGameStart();
	}

	float API_Time::getTimeSinceGameStartScaled()
	{
		return Time::getTimeSinceGameStartScaled();
	}

	float API_Time::getTimeSinceLevelStart()
	{
		return Time::getTimeSinceLevelStart();
	}

	float API_Time::getTimeSinceLevelStartScaled()
	{
		return Time::getTimeSinceLevelStartScaled();
	}
}