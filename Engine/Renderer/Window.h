#pragma once

#include <vector>
#include <functional>
#include <string>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace GX
{
	class Window
	{
	protected:
		bool running = true;
		bool closedByUser = false;
		void* sdlWindow;
		void* context = nullptr;
		
		std::function<void()> updateCallback = nullptr;

		void resize();

	public:
		void init(std::string title, int x, int y, int width, int height, uint32_t flags);
		bool run();
		
		void setUpdateCallback(std::function<void()> callback) { updateCallback = callback; }
		std::function<void()> getUpdateCallback() { return updateCallback; }

		bool isRunning() { return running; }
		int getWidth();
		int getHeight();
		void close();
		bool closeByUser();

		virtual void onResize(int width, int height) {}
		virtual void onRestore() {}
		virtual bool onClose() { return true; }
		
		void* getSdlWindow() { return sdlWindow; }
	#ifdef _WIN32
		HWND getHwnd();
	#else
		void* getHwnd();
	#endif
	};
}