#pragma once

#include <string>
#include <functional>

#include "../Engine/Assets/Font.h"

namespace GX
{
	class DialogProgress
	{
	public:
		void show();
		void hide();

		void setStatusText(std::string text, int barIdx = 0);
		void setProgress(float p, int barIdx = 0);
		void setTitle(std::string text);

		void setTwoProgressBars(bool value);
		bool getTwoProgressBars() { return twoProgressBars; }

	private:
		void createSecondProgressBar();
		void destroySecondProgressBar();

	#ifndef _WIN32
		static void* fontSurf;
		static void* fontData;
		static std::map<int, GlyphInfo> glyphs;

		void drawText(std::string text, int x, int y);
		void updateWindow();
	#endif

		std::string title = "Progress";
		std::string statusText1;
		std::string statusText2;
		float progress1 = 0;
		float progress2 = 0;

		bool twoProgressBars = false;
		bool visible = false;

		void* hwnd = nullptr;
		void* progress1Hwnd = nullptr;
		void* progress2Hwnd = nullptr;
		void* text1Hwnd = nullptr;
		void* text2Hwnd = nullptr;
	};
}