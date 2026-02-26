#pragma once

#include <string>
#include <iostream>

namespace GX
{
	class Helper
	{
	public:

		static std::string ExePath();
		static std::string ExeName();
		static std::string WorkingDir();
		static std::string getVersion();
	};
}