#include "Window.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <misc/cpp/imgui_stdlib.h>

#include "../Engine/UI/imgui_impl_sdl.h"
#include "../Engine/UI/imgui_impl_bgfx.h"
#include "../Engine/Classes/IO.h"
#include "../Engine/Classes/Helpers.h"
#include "../Engine/Renderer/Renderer.h"
#include "../Engine/Core/Engine.h"
#include "../Engine/Core/PhysicsManager.h"
#include "../Engine/Core/SoundManager.h"
#include "../Engine/Core/NavigationManager.h"
#include "../Engine/Core/APIManager.h"
#include "../Engine/Core/InputManager.h"
#include "../Engine/Assets/Asset.h"
#include "../Engine/Assets/Scene.h"
#include "../Engine/Core/Debug.h"
#include "../Engine/Core/Time.h"

#ifndef _WIN32
#include <unistd.h>
#endif

#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>

namespace GX
{
	void Window::init(std::string title, int x, int y, int width, int height, uint32_t flags)
	{
		Engine::getSingleton()->getSettings()->load();

		SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER);

		sdlWindow = SDL_CreateWindow(title.c_str(), x, y, width, height, flags | SDL_WINDOW_OPENGL);
		context = SDL_GL_CreateContext((SDL_Window*)sdlWindow);

		if (Engine::getSingleton()->getSettings()->getVSync())
			SDL_GL_SetSwapInterval(1);
		else
			SDL_GL_SetSwapInterval(0);

		SDL_Surface* windSurface = SDL_GetWindowSurface((SDL_Window*)sdlWindow);
		SDL_FillRect(windSurface, NULL, SDL_MapRGB(windSurface->format, 30, 30, 30));
		SDL_UpdateWindowSurface((SDL_Window*)sdlWindow);

		if (Engine::getSingleton()->getIsRuntimeMode())
			Debug::createLogFile(Helper::ExePath() + "player.log");
		else
			Debug::createLogFile(Helper::ExePath() + "editor.log");

		Engine::getSingleton()->loadPlugins();
		Renderer::getSingleton()->init(sdlWindow, context);
		
		if (!Engine::getSingleton()->getAssetsPath().empty())
		{
			PhysicsManager::getSingleton()->init();
			SoundManager::getSingleton()->init();
			NavigationManager::getSingleton()->init();
			APIManager::getSingleton()->init();

			Debug::log("\n---------------------------------\n");

			APIManager::getSingleton()->open();
		}
	}

	void Window::resize()
	{
		int w = getWidth();
		int h = getHeight();

		Renderer::getSingleton()->setSize(w, h);
		onResize(w, h);

		if (Engine::getSingleton()->getIsRuntimeMode())
		{
			void* args[2] = { &w, &h };
			APIManager::getSingleton()->execute("OnWindowResized", args, "int,int");
		}
	}

	bool Window::run()
	{
		ProjectSettings* settings = Engine::getSingleton()->getSettings();

		Time::resetTimeSinceLevelStart();

		bool engineRunning = !Engine::getSingleton()->shouldExit;

		SDL_Event event;
		while (running && engineRunning)
		{
			while (SDL_PollEvent(&event))
			{
				Renderer::getSingleton()->uiProcessEvent(&event);

				switch (event.type)
				{
				case SDL_QUIT:
					closeByUser();
					break;

				case SDL_WINDOWEVENT:
				{
					const SDL_WindowEvent& wev = event.window;
					switch (wev.event)
					{
					case SDL_WINDOWEVENT_RESIZED:
					case SDL_WINDOWEVENT_SIZE_CHANGED:
						resize();
						break;
					case SDL_WINDOWEVENT_RESTORED:
					case SDL_WINDOWEVENT_FOCUS_GAINED:
					{
						onRestore();
						break;
					}

					case SDL_WINDOWEVENT_CLOSE:
						//close();
						break;
					}
				} break;
				}

				InputManager::getSingleton()->updateKeys(&event);
			}

			engineRunning = !Engine::getSingleton()->shouldExit;

			InputManager::getSingleton()->updateMouse();

			Time::updateTime();

			if (updateCallback != nullptr)
				updateCallback();

			SoundManager::getSingleton()->update();
			if (!Scene::getLoadedScene().empty())
				PhysicsManager::getSingleton()->update();
			NavigationManager::getSingleton()->update();

			APIManager::getSingleton()->fixedUpdate();
			APIManager::getSingleton()->update();

			Renderer::getSingleton()->renderFrame();

			InputManager::getSingleton()->reset();

			/////

			if (!(SDL_GetWindowFlags((SDL_Window*)sdlWindow) & SDL_WINDOW_INPUT_FOCUS))
			{
	#ifdef _WIN32
				Sleep(200);
	#else
				usleep(200 * 1000);
	#endif
			}
		}

		APIManager::getSingleton()->execute("OnApplicationQuit");

		Engine::getSingleton()->clear();
		SoundManager::getSingleton()->destroy();
		PhysicsManager::getSingleton()->free();
		NavigationManager::getSingleton()->cleanup();
		Asset::unloadAll();
		Renderer::getSingleton()->shutdown();
		Engine::getSingleton()->unloadPlugins();
		APIManager::getSingleton()->close();

		SDL_DestroyWindow((SDL_Window*)sdlWindow);
		SDL_Quit();

		Debug::infoCallback = nullptr;
		Debug::errorCallback = nullptr;
		Debug::warningCallback = nullptr;

		Debug::log("\n---------------------------------\n");
		Debug::log("Engine shutdown");

		return closedByUser;
	}

	int Window::getWidth()
	{
		int w = 0;
		int h = 0;

		SDL_GL_GetDrawableSize((SDL_Window*)sdlWindow, &w, &h);

		return w;
	}

	int Window::getHeight()
	{
		int w = 0;
		int h = 0;

		SDL_GL_GetDrawableSize((SDL_Window*)sdlWindow, &w, &h);

		return h;
	}

	void Window::close()
	{
		running = false;
	}

	bool Window::closeByUser()
	{
		bool result = onClose();

		if (result)
		{
			running = false;
			closedByUser = true;
		}

		return result;
	}

	#ifdef _WIN32
	HWND Window::getHwnd()
	{
		if (sdlWindow != nullptr)
		{
			SDL_SysWMinfo inf;
			SDL_GetWindowWMInfo((SDL_Window*)sdlWindow, &inf);

			return inf.info.win.window;
		}

		return HWND();
	}
	#else
	void* Window::getHwnd()
	{
		if (sdlWindow != nullptr)
		{
			SDL_SysWMinfo inf;
			SDL_GetWindowWMInfo((SDL_Window*)sdlWindow, &inf);

			return (void*)(uintptr_t)inf.info.x11.window;
		}

		return nullptr;
	}
	#endif
}