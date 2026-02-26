#include <iostream>
#include <string>
#include <functional>

namespace GX
{
	class Debug
	{
	public:
		static int DbgColorRed;
		static int DbgColorGreen;
		static int DbgColorBlue;
		static int DbgColorYellow;
		static int DbgColorWhite;
		static int DbgColorFuchsia;
		static int DbgColorAqua;

		static std::function<void(std::string log)> infoCallback;
		static std::function<void(std::string log)> warningCallback;
		static std::function<void(std::string log)> errorCallback;

		static std::string logFilePath;
		static void createLogFile(std::string filePath);

		static void log(std::string str, int color = 15, bool writeToFile = true);
		static void logInfo(std::string str, bool writeToFile = true);
		static void logWarning(std::string str, bool writeToFile = true);
		static void logError(std::string str, bool writeToFile = true);
	};
}