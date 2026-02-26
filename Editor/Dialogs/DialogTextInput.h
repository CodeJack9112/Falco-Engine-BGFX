#pragma once

#include <string>
#include <functional>

namespace GX
{
	class DialogTextInput
	{
	public:
		DialogTextInput();
		~DialogTextInput();

		void show(std::string txt, std::string val, std::function<void(std::string val, bool okPressed)> callback);
		void update();

	private:
		std::string text = "Enter text:";
		std::string value = "";
		bool visible = false;

		std::function<void(std::string val, bool okPressed)> onCloseCallback = nullptr;
	};
}