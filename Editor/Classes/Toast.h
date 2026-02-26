#pragma once

#include <string>
#include <vector>

namespace GX
{
	class Texture;

	enum ToastIcon { TI_NONE, TI_SAVE, TI_IMAGE, TI_COMPILE, TI_WARNING, TI_CLIPBOARD_ADD, TI_CAMERA };

	class Toast
	{
	private:
		class ToastMessage
		{
		public:
			std::string text = "";
			float time = 0.0f;
			bool open = true;
			ToastIcon icon = TI_NONE;
		};

		static std::vector<ToastMessage*> messages;
		static std::vector<Texture*> icons;

	public:
		static void init();
		static void update();
		static void showMessage(std::string text, ToastIcon icon = TI_NONE);
	};
}