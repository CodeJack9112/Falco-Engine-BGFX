#pragma once

#include <string>

namespace GX
{
	class Texture;

	class DialogAbout
	{
	public:
		DialogAbout();
		~DialogAbout();

		void show();
		void update();

	private:
		bool visible = false;
		Texture * logoTexture;

		bool GetVersionInfo(std::string filename, int& major, int& minor, int& build, int& revision);
	};
}