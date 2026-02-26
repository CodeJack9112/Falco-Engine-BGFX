#include "Debug.h"

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

#include "../Classes/IO.h"

namespace GX
{
	int Debug::DbgColorRed = 12;
	int Debug::DbgColorGreen = 10;
	int Debug::DbgColorBlue = 9;
	int Debug::DbgColorYellow = 14;
	int Debug::DbgColorWhite = 15;
	int Debug::DbgColorFuchsia = 13;
	int Debug::DbgColorAqua = 11;

	std::string Debug::logFilePath = "";

	std::function<void(std::string log)> Debug::infoCallback = nullptr;
	std::function<void(std::string log)> Debug::warningCallback = nullptr;
	std::function<void(std::string log)> Debug::errorCallback = nullptr;

	void Debug::createLogFile(std::string filePath)
	{
		logFilePath = filePath;
		IO::WriteText(logFilePath, "");
	}

	void Debug::log(std::string str, int color, bool writeToFile)
	{
		if (str.empty())
			return;

		//HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
		//SetConsoleTextAttribute(hConsole, color);

		//std::cout << str << std::endl;
		if (writeToFile)
			IO::AppendText(logFilePath, str + "\n");

		//SetConsoleTextAttribute(hConsole, DbgColorWhite);

		if (color == DbgColorRed)
		{
			if (errorCallback != nullptr)
				errorCallback(str);
		}

		if (color == DbgColorWhite)
		{
			if (infoCallback != nullptr)
				infoCallback(str);
		}

		if (color == DbgColorYellow)
		{
			if (warningCallback != nullptr)
				warningCallback(str);
		}
	}

	void Debug::logInfo(std::string str, bool writeToFile)
	{
		log(str, DbgColorAqua, writeToFile);
	}

	void Debug::logWarning(std::string str, bool writeToFile)
	{
		log(str, DbgColorYellow, writeToFile);
	}

	void Debug::logError(std::string str, bool writeToFile)
	{
		log(str, DbgColorRed, writeToFile);
	}
}