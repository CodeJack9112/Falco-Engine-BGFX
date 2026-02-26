#include "API_QualitySettings.h"

#include "../Engine/Core/Engine.h"
#include "../Renderer/Renderer.h"
#include "../Engine/Serialization/Settings/ProjectSettings.h"

#include <SDL2/SDL.h>

namespace GX
{
	void API_QualitySettings::setWindowSize(int width, int height)
	{
		if (width > 0 && height > 0)
		{
			ProjectSettings* projectSettings = Engine::getSingleton()->getSettings();
			projectSettings->setScreenWidth(width);
			projectSettings->setScreenHeight(height);

			SDL_Window* win = (SDL_Window*)Renderer::getSingleton()->getSdlWindow();
			SDL_SetWindowSize(win, width, height);
		}
	}

	int API_QualitySettings::getWindowWidth()
	{
		ProjectSettings* projectSettings = Engine::getSingleton()->getSettings();
		return projectSettings->getScreenWidth();
	}

	int API_QualitySettings::getWindowHeight()
	{
		ProjectSettings* projectSettings = Engine::getSingleton()->getSettings();
		return projectSettings->getScreenHeight();
	}

	void API_QualitySettings::setFullScreen(bool value)
	{
		ProjectSettings* projectSettings = Engine::getSingleton()->getSettings();
		projectSettings->setFullScreen(value);

		SDL_Window* win = (SDL_Window*)Renderer::getSingleton()->getSdlWindow();

		uint32_t flags = 0;
		if (value)
		{
			if (projectSettings->getExclusiveMode())
				flags |= SDL_WINDOW_FULLSCREEN;
			else
				flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
		}

		SDL_SetWindowFullscreen(win, flags);
	}

	bool API_QualitySettings::getFullScreen()
	{
		ProjectSettings* projectSettings = Engine::getSingleton()->getSettings();
		return projectSettings->getFullScreen();
	}

	void API_QualitySettings::setExclusiveMode(bool value)
	{
		ProjectSettings* projectSettings = Engine::getSingleton()->getSettings();
		projectSettings->setExclusiveMode(value);
	}

	bool API_QualitySettings::getExclusiveMode()
	{
		ProjectSettings* projectSettings = Engine::getSingleton()->getSettings();
		return projectSettings->getExclusiveMode();
	}

	void API_QualitySettings::setVSync(bool value)
	{
		ProjectSettings* projectSettings = Engine::getSingleton()->getSettings();
		projectSettings->setVSync(value);
	}

	bool API_QualitySettings::getVSync()
	{
		ProjectSettings* projectSettings = Engine::getSingleton()->getSettings();
		return projectSettings->getVSync();
	}

	void API_QualitySettings::setFXAA(bool value)
	{
		ProjectSettings* projectSettings = Engine::getSingleton()->getSettings();
		projectSettings->setFXAA(value);
	}

	bool API_QualitySettings::getFXAA()
	{
		ProjectSettings* projectSettings = Engine::getSingleton()->getSettings();
		return projectSettings->getFXAA();
	}

	void API_QualitySettings::setDynamicResolution(bool value)
	{
		ProjectSettings* projectSettings = Engine::getSingleton()->getSettings();
		projectSettings->setUseDynamicResolution(value);
	}

	bool API_QualitySettings::getDynamicResolution()
	{
		ProjectSettings* projectSettings = Engine::getSingleton()->getSettings();
		return projectSettings->getUseDynamicResolution();
	}

	void API_QualitySettings::setDynamicResolutionValue(float value)
	{
		ProjectSettings* projectSettings = Engine::getSingleton()->getSettings();
		projectSettings->setDynamicResolution(value);
	}

	float API_QualitySettings::getDynamicResolutionValue()
	{
		ProjectSettings* projectSettings = Engine::getSingleton()->getSettings();
		return projectSettings->getDynamicResolution();
	}

	void API_QualitySettings::setAutoDynamicResolution(bool value)
	{
		ProjectSettings* projectSettings = Engine::getSingleton()->getSettings();
		projectSettings->setAutoDynamicResolution(value);
	}

	bool API_QualitySettings::getAutoDynamicResolution()
	{
		ProjectSettings* projectSettings = Engine::getSingleton()->getSettings();
		return projectSettings->getAutoDynamicResolution();
	}

	void API_QualitySettings::setAutoDynamicResolutionTargetFPS(int value)
	{
		ProjectSettings* projectSettings = Engine::getSingleton()->getSettings();
		projectSettings->setAutoDynamicResolutionTargetFPS(value);
	}

	int API_QualitySettings::getAutoDynamicResolutionTargetFPS()
	{
		ProjectSettings* projectSettings = Engine::getSingleton()->getSettings();
		return projectSettings->getAutoDynamicResolutionTargetFPS();
	}

	void API_QualitySettings::setDirectionalShadowsQuality(int value)
	{
		ProjectSettings* projectSettings = Engine::getSingleton()->getSettings();
		int val = 128, i = 0;
		while (i < value)
		{
			val *= 2;
			++i;
		}
		projectSettings->setDirectionalShadowResolution(val);
	}

	int API_QualitySettings::getDirectionalShadowsQuality()
	{
		ProjectSettings* projectSettings = Engine::getSingleton()->getSettings();
		int val = 4096, i = 5;
		while (val > projectSettings->getDirectionalShadowResolution())
		{
			val /= 2;
			--i;
		}

		return i;
	}

	void API_QualitySettings::setPointShadowsQuality(int value)
	{
		ProjectSettings* projectSettings = Engine::getSingleton()->getSettings();
		int val = 128, i = 0;
		while (i < value)
		{
			val *= 2;
			++i;
		}
		projectSettings->setPointShadowResolution(val);
	}

	int API_QualitySettings::getPointShadowsQuality()
	{
		ProjectSettings* projectSettings = Engine::getSingleton()->getSettings();
		int val = 4096, i = 5;
		while (val > projectSettings->getPointShadowResolution())
		{
			val /= 2;
			--i;
		}

		return i;
	}

	void API_QualitySettings::setSpotShadowsQuality(int value)
	{
		ProjectSettings* projectSettings = Engine::getSingleton()->getSettings();
		int val = 128, i = 0;
		while (i < value)
		{
			val *= 2;
			++i;
		}
		projectSettings->setSpotShadowResolution(val);
	}

	int API_QualitySettings::getSpotShadowsQuality()
	{
		ProjectSettings* projectSettings = Engine::getSingleton()->getSettings();
		int val = 4096, i = 5;
		while (val > projectSettings->getSpotShadowResolution())
		{
			val /= 2;
			--i;
		}

		return i;
	}

	void API_QualitySettings::setShadowsCascadesCount(int value)
	{
		ProjectSettings* projectSettings = Engine::getSingleton()->getSettings();
		projectSettings->setShadowCascadesCount(value + 2);
	}

	int API_QualitySettings::getShadowsCascadesCount()
	{
		ProjectSettings* projectSettings = Engine::getSingleton()->getSettings();
		return projectSettings->getShadowCascadesCount() - 2;
	}

	void API_QualitySettings::setShadowsSofteness(int value)
	{
		ProjectSettings* projectSettings = Engine::getSingleton()->getSettings();
		if (value == 0) projectSettings->setShadowSamplesCount(1);
		if (value == 1) projectSettings->setShadowSamplesCount(2);
		if (value == 2) projectSettings->setShadowSamplesCount(4);
		if (value == 3) projectSettings->setShadowSamplesCount(6);
	}

	int API_QualitySettings::getShadowsSofteness()
	{
		ProjectSettings* projectSettings = Engine::getSingleton()->getSettings();
		if (projectSettings->getShadowSamplesCount() == 1) return 0;
		if (projectSettings->getShadowSamplesCount() == 2) return 1;
		if (projectSettings->getShadowSamplesCount() == 4) return 2;
		if (projectSettings->getShadowSamplesCount() == 6) return 3;

		return 1;
	}

	void API_QualitySettings::setShadowsDistance(int value)
	{
		ProjectSettings* projectSettings = Engine::getSingleton()->getSettings();
		projectSettings->setShadowDistance(value);
	}

	int API_QualitySettings::getShadowsDistance()
	{
		ProjectSettings* projectSettings = Engine::getSingleton()->getSettings();
		return projectSettings->getShadowDistance();
	}

	void API_QualitySettings::setShadowsEnabled(bool value)
	{
		ProjectSettings* projectSettings = Engine::getSingleton()->getSettings();
		projectSettings->setShadowsEnabled(value);
	}

	int API_QualitySettings::getShadowsEnabled()
	{
		ProjectSettings* projectSettings = Engine::getSingleton()->getSettings();
		return projectSettings->getShadowsEnabled();
	}
}