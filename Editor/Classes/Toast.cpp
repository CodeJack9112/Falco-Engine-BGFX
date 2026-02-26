#include "Toast.h"

#include <algorithm>

#include <imgui.h>
#include <imgui_internal.h>

#include "../Engine/glm/vec2.hpp"
#include "../Engine/Assets/Texture.h"
#include "../Engine/Classes/Helpers.h"
#include "../Engine/Core/Engine.h"
#include "../Engine/Core/Time.h"
#include "../Engine/Renderer/Renderer.h"

#include "../Windows/MainWindow.h"

namespace GX
{
	std::vector<Toast::ToastMessage*> Toast::messages;
	std::vector<Texture*> Toast::icons;

	void Toast::init()
	{
		ToastMessage* dummy = new ToastMessage();
		dummy->time = 1.0f;
		dummy->text = "##dummy";
		messages.push_back(dummy);

		icons.push_back(Texture::load(Helper::ExePath(), "Editor/Icons/Toolbar/save.png", false, Texture::CompressionMethod::None, true));
		icons.push_back(Texture::load(Helper::ExePath(), "Editor/Icons/Assets/texture.png", false, Texture::CompressionMethod::None, true));
		icons.push_back(Texture::load(Helper::ExePath(), "Editor/Icons/Toolbar/compile.png", false, Texture::CompressionMethod::None, true));
		icons.push_back(Texture::load(Helper::ExePath(), "Editor/Icons/Other/warning.png", false, Texture::CompressionMethod::None, true));
		icons.push_back(Texture::load(Helper::ExePath(), "Editor/Icons/Other/clipboard.png", false, Texture::CompressionMethod::None, true));
		icons.push_back(Texture::load(Helper::ExePath(), "Editor/Icons/Toolbar/camera.png", false, Texture::CompressionMethod::None, true));
	}

	void Toast::showMessage(std::string text, ToastIcon icon)
	{
		ToastMessage * msg = new ToastMessage();
		msg->text = text;
		msg->time = 20.0f;
		msg->icon = icon;

		if (messages.size() > 0)
			messages.insert(messages.begin() + 1, msg);
	}

	void Toast::update()
	{
		std::vector<ToastMessage*> del;

		glm::vec2 wsz = glm::vec2(MainWindow::getSingleton()->getWidth(), MainWindow::getSingleton()->getHeight());

		int i = 0;
		if (messages.size() > 0)
		{
			for (auto it = messages.begin(); it != messages.end(); ++it, ++i)
			{
				ToastMessage* msg = *it;

				if (msg->time > 0.0f)
				{
					if (msg->text != "##dummy")
						msg->time -= 2.0f * Time::getDeltaTime();

					ImGui::SetNextWindowSize(ImVec2(0, 0));
					ImGui::SetNextWindowPos(ImVec2(10, wsz.y - 42 * i));

					if (msg->time < 5.0f)
					{
						float a = 1.0f / 5.0f * msg->time;
						ImGui::PushStyleVar(ImGuiStyleVar_Alpha, a);
					}

					ImGui::Begin(("##toastMsg_" + std::to_string(i)).c_str(), &msg->open, ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
					if (msg->icon != TI_NONE)
					{
						ImGui::Image((void*)icons[static_cast<int>(msg->icon) - 1]->getHandle().idx, ImVec2(16, 16), ImVec2(0, 1), ImVec2(1, 0));
						ImGui::SameLine();
					}
					ImGui::Text(msg->text.c_str());
					ImGui::End();

					if (msg->time < 5.0f)
						ImGui::PopStyleVar();
				}
				else
				{
					del.push_back(msg);
				}
			}
		}

		for (auto it = del.begin(); it != del.end(); ++it)
		{
			auto d = std::find(messages.begin(), messages.end(), *it);
			if (d != messages.end())
				messages.erase(d);
		}

		del.clear();
	}
}