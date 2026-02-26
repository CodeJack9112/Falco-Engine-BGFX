#include "DialogAbout.h"

#include <imgui.h>
#include <imgui_internal.h>

#include "../Windows/MainWindow.h"
#include "../Engine/Classes/Helpers.h"
#include "../Engine/Classes/StringConverter.h"
#include "../Engine/Assets/Texture.h"

#ifdef _WIN32
#include <shellapi.h>
#endif

namespace GX
{
	ImVec4 linkColor = ImVec4(0.4f, 0.4f, 1.0f, 1.0f);
	std::string version = "";

	DialogAbout::DialogAbout()
	{
		
	}

	DialogAbout::~DialogAbout()
	{
	}

	void DialogAbout::show()
	{
		logoTexture = Texture::load(Helper::ExePath(), "Editor/Logo.png", false, Texture::CompressionMethod::None, true);
		visible = true;

		version = Helper::getVersion();
	}

	void DialogAbout::update()
	{
		if (!visible)
			return;

		ImGui::SetNextWindowSize(ImVec2(300.0f, 230.0f));
		ImGui::SetNextWindowPos(ImVec2(MainWindow::getSingleton()->getWidth() / 2 - 150.0f, MainWindow::getSingleton()->getHeight() / 2 - 150.0f), ImGuiCond_FirstUseEver, ImVec2(0.5f, 0.5f));
		ImGui::Begin("About Falco Engine", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoDocking);

		ImGui::Image((void*)logoTexture->getHandle().idx, ImVec2(64, 64), ImVec2(0, 1), ImVec2(1, 0));
		ImGui::Text("Falco Engine, version"); ImGui::SameLine(); ImGui::TextColored(ImVec4(0.9f, 0.9f, 0.0f, 1.0f), version.c_str());
		ImGui::Dummy(ImVec2(100, 10));
		/*ImGui::Text("Authors:");
		ImGui::Text("Alexander Gerashchenko (tracer0707)");
		ImGui::Text("Valeriy Sokolov (wsokolov)");
		ImGui::Dummy(ImVec2(100, 10));*/
		ImGui::Text("Copyright: Falco Software Company");
		ImGui::TextColored(linkColor, "https://www.falco3d.com");
		if (ImGui::IsItemHovered())
			linkColor = ImVec4(0.7f, 0.7f, 1.0f, 1.0f);
		else
			linkColor = ImVec4(0.4f, 0.4f, 1.0f, 1.0f);

		if (ImGui::IsItemClicked())
		{
	#ifdef _WIN32
			ShellExecute(0, 0, u8"https://www.falco3d.com", 0, 0, SW_SHOW);
	#endif
		}

		ImGui::Dummy(ImVec2(100, 10));

		ImGui::Text("(C) 2018 - 2024");

		ImGui::Dummy(ImVec2(0.0f, 10.0f));

		if (ImGui::Button("Close"))
		{
			visible = false;
		}

		ImGui::End();
	}
}