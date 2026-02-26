#include "ConsoleWindow.h"

#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>
#include <regex>

#include <imgui.h>
#include <imgui_internal.h>
#include <dear-imgui/misc/cpp/imgui_stdlib.h>

#include "MainWindow.h"
#include "InspectorWindow.h"

#include "../Serialization/EditorSettings.h"
#include "../Engine/Core/Engine.h"
#include "../Engine/Core/APIManager.h"
#include "../Engine/Core/Debug.h"
#include "../Engine/Assets/Texture.h"
#include "../Engine/UI/ImGUIWidgets.h"
#include "../Engine/Classes/StringConverter.h"

namespace GX
{
	ConsoleWindow::LogMessage::LogMessage(std::string msg, Color col, LogMessageType msgType)
	{
		int p = msg.find('\n');

		if (p != std::string::npos)
			header = msg.substr(0, p);
		else
			header = msg;

		if (header.length() > 512)
			header = header.substr(0, 511) + "...";

		text = msg;
		color = col;
		type = msgType;
	}

	ConsoleWindow::ConsoleWindow()
	{
	}

	ConsoleWindow::~ConsoleWindow()
	{
	}

	void ConsoleWindow::init()
	{
		APIManager::getSingleton()->setLogCallback([=](std::string msg)
			{
				log(CP_UNI(msg));
			}
		);

		Debug::infoCallback = [=](std::string src) { log(src, LogMessageType::LMT_INFO); };
		Debug::warningCallback = [=](std::string src) { log(src, LogMessageType::LMT_WARNING); };
		Debug::errorCallback = [=](std::string src) { log(src, LogMessageType::LMT_ERROR); };

		clearOnPlay = MainWindow::getSettings()->clearConsoleOnPlay;
		showErrors = MainWindow::getSettings()->consoleShowErrors;
		showWarnings = MainWindow::getSettings()->consoleShowWarnings;
		showInfos = MainWindow::getSettings()->consoleShowInfos;

		iconError = MainWindow::loadEditorIcon("Other/error.png");
		iconWarning = MainWindow::loadEditorIcon("Other/warning.png");
		iconInfo = MainWindow::loadEditorIcon("Other/info.png");
		iconClear = MainWindow::loadEditorIcon("Other/clear.png");
		iconCompile = MainWindow::loadEditorIcon("Toolbar/compile.png");
	}

	void ConsoleWindow::update()
	{
		if (opened)
		{
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

			if (ImGui::Begin("Console", &opened, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
			{
				ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(5, 5));
				ImGui::BeginChild("Console_buttons", ImVec2(-1.0f, 30.0f), false, ImGuiWindowFlags_AlwaysUseWindowPadding | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
				{
					if (ImGui::ImageButtonWithText((void*)iconClear->getHandle().idx, "Clear", ImVec2(16, 16),
						ImVec2(0, 1), ImVec2(1, 0), ImVec2(-1, -1), -1, ImVec4(0, 0, 0, 0),
						ImVec4(1, 1, 1, 1), "##clear_console"))
					{
						clear();
					}

					ImGui::SameLine();

					if (ImGui::Checkbox("Clear on play", &clearOnPlay))
					{
						EditorSettings* settings = MainWindow::getSettings();
						settings->clearConsoleOnPlay = clearOnPlay;
						settings->save();
					}

					ImGui::SameLine();
					ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
					ImGui::SameLine();

					if (ImGui::ImageButtonWithText((void*)iconCompile->getHandle().idx, "Compile scripts", ImVec2(16, 16),
						ImVec2(0, 1), ImVec2(1, 0), ImVec2(-1, -1), -1, ImVec4(0, 0, 0, 0),
						ImVec4(1, 1, 1, 1), "##compile_scripts"))
					{
						MainWindow::addOnEndUpdateCallback([=]()
							{
								APIManager::getSingleton()->compile(APIManager::CompileConfiguration::Debug);
								MainWindow::getSingleton()->getInspectorWindow()->updateCurrentEditor();
							}
						);
					}

					ImGui::SameLine();
					ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
					ImGui::SameLine();

					ImVec4 col = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
					ImVec4 col1 = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
					ImVec4 col2 = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
					ImVec4 col3 = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);

					if (!showErrors) col1 = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
					if (!showWarnings) col2 = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
					if (!showInfos) col3 = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);

					int num_errors = 0;
					int num_warnings = 0;
					int num_infos = 0;

					for (auto ld : logData)
					{
						if (ld.type == LMT_ERROR)
							num_errors++;
						else if (ld.type == LMT_WARNING)
							num_warnings++;
						else if (ld.type == LMT_INFO)
							num_infos++;
					}

					std::string str_num_errors = std::to_string(num_errors);
					std::string str_num_warnings = std::to_string(num_warnings);
					std::string str_num_infos = std::to_string(num_infos);

					if (ImGui::ImageButtonWithText((void*)iconError->getHandle().idx, str_num_errors.c_str(), ImVec2(16, 16),
						ImVec2(0, 1), ImVec2(1, 0), ImVec2(-1, -1), -1, col, col1, "##console_errors"))
					{
						showErrors = !showErrors;
						EditorSettings* settings = MainWindow::getSettings();
						settings->consoleShowErrors = showErrors;
						settings->save();
					}

					MainWindow::HelpMarker("Show errors");

					ImGui::SameLine();

					if (ImGui::ImageButtonWithText((void*)iconWarning->getHandle().idx, str_num_warnings.c_str(), ImVec2(16, 16),
						ImVec2(0, 1), ImVec2(1, 0), ImVec2(-1, -1), -1, col, col2, "##console_warnings"))
					{
						showWarnings = !showWarnings;
						EditorSettings* settings = MainWindow::getSettings();
						settings->consoleShowWarnings = showWarnings;
						settings->save();
					}

					MainWindow::HelpMarker("Show warnings");

					ImGui::SameLine();

					if (ImGui::ImageButtonWithText((void*)iconInfo->getHandle().idx, str_num_infos.c_str(), ImVec2(16, 16),
						ImVec2(0, 1), ImVec2(1, 0), ImVec2(-1, -1), -1, col, col3, "##console_infos"))
					{
						showInfos = !showInfos;
						EditorSettings* settings = MainWindow::getSettings();
						settings->consoleShowInfos = showInfos;
						settings->save();
					}

					MainWindow::HelpMarker("Show infos");
				}
				ImGui::EndChild();
				ImGui::PopStyleVar();

				ImGui::SeparatorEx(ImGuiSeparatorFlags_Horizontal);

				ImGui::BeginChild("ConsoleControls", ImVec2(-1, -1), false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
				{
					ImGuiWindow* wnd = ImGui::GetCurrentWindow();

					ImVec2 sz0 = ImVec2(-1, -1);
					bool showSplitter = true;

					if (current_item > -1 && current_item < 999)
					{
						if (cntSize1 > wnd->Size.y * 0.8f)
							cntSize1 = wnd->Size.y * 0.8f;

						if (cntSize1 < wnd->Size.y * 0.5f)
							cntSize1 = wnd->Size.y * 0.5f;

						ImGui::Splitter(false, 4.0f, &cntSize1, &cntSize2, 10.0f, 10.0f, -1.0f, "##Console_content_splitter");
						
						sz0.y = cntSize1;
					}

					ImGui::BeginChild("ConsoleVS", sz0);
					{
						int i = 0;
						for (auto it = logData.begin(); it != logData.end(); ++it, ++i)
						{
							LogMessage& msg = *it;

							if (msg.type == LogMessageType::LMT_ERROR)
							{
								if (!showErrors)
									continue;
							}
							else if (msg.type == LogMessageType::LMT_WARNING)
							{
								if (!showWarnings)
									continue;
							}
							else if (msg.type == LogMessageType::LMT_INFO)
							{
								if (!showInfos)
									continue;
							}

							bool item_selected = (i == current_item);

							if (ImGui::Selectable(("##logText_" + std::to_string(i)).c_str(), item_selected))
							{
								current_item = i;
							}

							if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
							{
								ImGui::OpenPopup("logTextPopup");
								current_item = i;
							}

							ImGui::SameLine();
							ImGui::TextColored(ImVec4(it->color.r(), it->color.g(), it->color.b(), it->color.a()), it->header.c_str());
						}

						ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(5, 5));
						if (ImGui::BeginPopupContextWindow("logTextPopup"))
						{
							if (ImGui::MenuItem("Copy", ""))
							{
								if (current_item > -1 && current_item < 999)
									ImGui::SetClipboardText(logData[current_item].text.c_str());
							}

							ImGui::EndPopup();
						}
						ImGui::PopStyleVar();

						ImGui::Text("");

						if (scrollTo)
						{
							ImGui::SetScrollHereY();
							scrollTo = false;
						}
					}
					ImGui::EndChild();

					if (current_item > -1 && current_item < 999)
					{
						ImGui::BeginChild("ConsoleVS_text", ImVec2(-1, -1));
						{
							ImGui::Dummy(ImVec2(0, 2));

							LogMessage& msg = logData[current_item];
							
							ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(msg.color.r() * 255.0f, msg.color.g() * 255.0f, msg.color.b() * 255.0f, msg.color.a() * 255.0f));
							ImGui::InputTextMultiline("##log_message", &msg.text, ImVec2(-1, -1), ImGuiInputTextFlags_ReadOnly);
							ImGui::PopStyleColor();
						}
						ImGui::EndChild();
					}
				}
				ImGui::EndChild();
			}

			ImGui::End();

			ImGui::PopStyleVar();
		}
	}

	void ConsoleWindow::log(std::string value, LogMessageType type)
	{
		if (value.empty())
			return;

		show(true);

		LogMessageType _type = type;

		Color color = Color(1.0f, 1.0f, 1.0f, 1.0f);

		if (type == LMT_AUTODETECT)
		{
			boost::regex xRegEx(".*warning[[:space:]]CS\d*.*");
			boost::smatch xResults;

			if (boost::regex_match(value, xResults, xRegEx))
			{
				for (int i = 0; i < xResults.length(); ++i)
				{
					if (xResults[i].length() > 0)
						_type = LMT_WARNING;
				}
			}

			xRegEx = boost::regex(".*error[[:space:]]CS\d*.*");

			if (boost::regex_match(value, xResults, xRegEx))
			{
				for (int i = 0; i < xResults.length(); ++i)
				{
					if (xResults[i].length() > 0)
						_type = LMT_ERROR;
				}
			}
		}

		if (_type == LMT_INFO || _type == LMT_AUTODETECT)
			color = Color(1.0f, 1.0f, 1.0f, 1.0f);
		if (_type == LMT_WARNING)
			color = Color(1.0f, 1.0f, 0.0f, 1.0f);
		if (_type == LMT_ERROR)
			color = Color(1.0f, 0.0f, 0.0f, 1.0f);

		logData.push_back(LogMessage(value, color, _type));

		if (logData.size() > 999)
			logData.erase(logData.begin());

		scrollTo = true;
	}

	void ConsoleWindow::clear()
	{
		current_item = -1;
		logData.clear();
	}
}