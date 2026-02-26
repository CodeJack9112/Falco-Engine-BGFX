#include "DialogTextInput.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <dear-imgui/misc/cpp/imgui_stdlib.h>

#include "../Windows/MainWindow.h"

namespace GX
{
	DialogTextInput::DialogTextInput()
	{
	}

	DialogTextInput::~DialogTextInput()
	{
	}

	void DialogTextInput::show(std::string txt, std::string val, std::function<void(std::string val, bool okPressed)> callback)
	{
		text = txt;
		value = val;
		onCloseCallback = callback;
		visible = true;
	}

	void DialogTextInput::update()
	{
		if (!visible)
			return;

		ImGui::SetNextWindowSize(ImVec2(300.0f, 180.0f));
		ImGui::SetNextWindowPos(ImVec2(MainWindow::getSingleton()->getWidth() / 2, MainWindow::getSingleton()->getHeight() / 2), ImGuiCond_FirstUseEver, ImVec2(0.5f, 0.5f));
		ImGui::Begin(text.c_str(), nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoDocking);

		ImGui::InputText("##dialogPromptText", &value);

		bool closed = false;
		bool ok = false;

		if (ImGui::Button("Ok"))
		{
			closed = true;
			ok = true;
		}

		ImGui::SameLine();

		if (ImGui::Button("Cancel"))
		{
			closed = true;
			ok = false;
		}

		ImGui::End();

		if (closed)
		{
			if (onCloseCallback != nullptr)
			{
				onCloseCallback(value, ok);
			}
		}
	}
}