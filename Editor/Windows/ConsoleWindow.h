#pragma once

#include <string>
#include <vector>
#include "../Engine/Renderer/Color.h"

namespace GX
{
	enum LogMessageType { LMT_INFO, LMT_WARNING, LMT_ERROR, LMT_AUTODETECT };

	class Texture;

	class ConsoleWindow
	{
	private:
		struct LogMessage
		{
		public:
			std::string header = "";
			std::string text = "";
			Color color = Color(1.0f, 1.0f, 1.0f, 1.0f);
			LogMessageType type = LogMessageType::LMT_AUTODETECT;

			LogMessage() = default;
			LogMessage(std::string msg, Color col = Color(1.0f, 1.0f, 1.0f, 1.0f), LogMessageType msgType = LogMessageType::LMT_AUTODETECT);
		};

		bool opened = true;
		bool clearOnPlay = true;
		bool scrollTo = false;

		bool showErrors = true;
		bool showWarnings = true;
		bool showInfos = true;

		int current_item = -1;
		float cntSize1 = 100.0f;
		float cntSize2 = 64.0f;

		std::vector<LogMessage> logData;

		Texture* iconError = nullptr;
		Texture* iconWarning = nullptr;
		Texture* iconInfo = nullptr;
		Texture* iconClear = nullptr;
		Texture* iconCompile = nullptr;

	public:
		ConsoleWindow();
		~ConsoleWindow();

		void init();
		void update();
		void show(bool show) { opened = show; }
		void clear();

		bool getVisible() { return opened; }
		
		void log(std::string value, LogMessageType type = LMT_AUTODETECT);

		void setClearOnPlay(bool value) { clearOnPlay = value; }
		bool getClearOnPlay() { return clearOnPlay; }

		void setShowErrors(bool value) { showErrors = value; }
		bool getShowErrors() { return showErrors; }

		void setShowWarnings(bool value) { showWarnings = value; }
		bool getShowWarnings() { return showWarnings; }

		void setShowInfos(bool value) { showInfos = value; }
		bool getShowInfos() { return showInfos; }

		void setScrollToBottom(bool val) { scrollTo = val; }
	};
}