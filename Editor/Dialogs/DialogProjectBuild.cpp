#include "DialogProjectBuild.h"

#include <imgui.h>
#include <boost/algorithm/string.hpp>

#include "../Engine/Core/Engine.h"
#include "../Engine/Classes/IO.h"
#include "../Engine/Classes/StringConverter.h"

#include "../Windows/MainWindow.h"
#include "../Classes/TreeNode.h"
#include "../Classes/TreeView.h"
#include "../BuildSystems/BuildSystemWin64.h"
#include "../BuildSystems/ProjectOptimizer.h"

#ifdef _WIN32
#include <shobjidl_core.h>
#endif

namespace GX
{
	DialogProjectBuild::DialogProjectBuild()
	{
		projectOptimizer = new ProjectOptimizer();
	}

	DialogProjectBuild::~DialogProjectBuild()
	{
		delete projectOptimizer;
	}

	void DialogProjectBuild::show()
	{
		visible = true;
	}

	void DialogProjectBuild::hide()
	{
		visible = false;
	}

	void DialogProjectBuild::update()
	{
		if (!visible)
			return;

		ProjectSettings* settings = Engine::getSingleton()->getSettings();

		ImGui::SetNextWindowSize(ImVec2(320, 375));
		if (ImGui::Begin("Build project", &visible, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking))
		{
			ImGui::Text("Target platform");
			ImGui::PushItemWidth(-1);

			std::vector<std::string> platforms =
			{
				"Windows x64",
				//"Linux x64"
			};

			if (ImGui::BeginCombo("##target_platform", platforms[currentPlatform].c_str()))
			{
				if (ImGui::Selectable(platforms[0].c_str(), currentPlatform == 0))
				{
					currentPlatform = 0;
				}

				/*if (ImGui::Selectable(platforms[1].c_str(), currentPlatform == 1))
				{
					currentPlatform = 1;
				}*/

				ImGui::EndCombo();
			}

			ImGui::Dummy(ImVec2(0, 10));

			ImGui::Text("Scenes in build");
			if (ImGui::BeginChild("##scenes_in_build", ImVec2(304, 200), true))
			{
				int remove_scene = -1;
				int s = 0;
				for (auto& scene : settings->getScenes())
				{
					if (ImGui::Selectable(scene.c_str(), s == selScene))
						selScene = s;

					if (ImGui::BeginPopupContextItem(("scene_popup_" + std::to_string(s)).c_str(), 1))
					{
						if (ImGui::Selectable("Remove"))
							remove_scene = s;

						ImGui::EndPopup();
					}

					++s;
				}

				if (remove_scene > -1)
				{
					auto& scenes = settings->getScenes();
					if (settings->getStartupScene() == scenes[remove_scene])
						settings->setStartupScene("");
					scenes.erase(scenes.begin() + remove_scene);
					settings->save();
				}
			}
			ImGui::EndChild();

			if (ImGui::BeginDragDropTarget())
			{
				const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_TreeView");
				if (payload != nullptr)
				{
					TreeNode* move_from = (TreeNode*)payload->Data;
					if (move_from->treeView->getTag() == "Assets")
					{
						std::string path = move_from->getPath();
						std::string ext = IO::GetFileExtension(path);
						if (ext == "scene")
						{
							settings->getScenes().push_back(path);
							settings->save();
						}
					}
				}
			}

			std::string startScene = settings->getStartupScene();
			ImGui::Text("Startup scene");
			ImGui::PushItemWidth(-1);
			if (ImGui::BeginCombo("##start_scene", startScene.c_str()))
			{
				for (auto& scene : settings->getScenes())
				{
					if (ImGui::Selectable(scene.c_str(), startScene == scene))
					{
						startScene = scene;
						settings->setStartupScene(scene);
						settings->save();
					}
				}
				ImGui::EndCombo();
			}

			ImGui::Dummy(ImVec2(0, 10));

			ImGui::SetCursorPosX(176);

			if (ImGui::Button("Project settings"))
			{
				MainWindow::getSingleton()->onProjectSettings();
			}

			ImGui::SameLine();

			if (ImGui::Button("Build"))
			{
	#ifdef _WIN32
				IFileDialog* pfd;
				if (SUCCEEDED(CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd))))
				{
					DWORD dwOptions;
					if (SUCCEEDED(pfd->GetOptions(&dwOptions)))
					{
						pfd->SetOptions(dwOptions | FOS_PICKFOLDERS);
					}
					if (SUCCEEDED(pfd->Show(NULL)))
					{
						IShellItem* psi;
						if (SUCCEEDED(pfd->GetResult(&psi)))
						{
							LPWSTR g_path = NULL;
							LPWSTR g_name = NULL;

							if (!SUCCEEDED(psi->GetDisplayName(SIGDN_DESKTOPABSOLUTEPARSING, &g_path)))
							{
								MessageBox(NULL, "SIGDN_DESKTOPABSOLUTEPARSING failed", NULL, NULL);
							}
							else
							{
								if (!SUCCEEDED(psi->GetDisplayName(SIGDN_NORMALDISPLAY, &g_name)))
								{
									MessageBox(NULL, "SIGDN_NORMALDISPLAY failed", NULL, NULL);
								}
								else
								{
									std::string filePathName = StringConvert::ws2s(g_name);
									std::string filePath = StringConvert::ws2s(g_path);

									filePath = boost::replace_all_copy(filePath, "\\", "/");
									if (filePath.at(filePath.length() - 1) != '/')
										filePath = filePath + "/";

									//Build project
									if (!IO::DirExists(filePath))
										IO::CreateDir(filePath, true);

									BuildSystemWin64::Platform platform = static_cast<BuildSystemWin64::Platform>(currentPlatform);

									MainWindow::addOnEndUpdateCallback([=]()
										{
											auto assets = projectOptimizer->getUsedResources();
											BuildSystemWin64::build(platform, Engine::getSingleton()->getAppName(), filePath, assets);
										}
									);
								}
							}

							CoTaskMemFree(g_path);
							CoTaskMemFree(g_name);

							psi->Release();
						}
					}
					pfd->Release();
				}
	#else
				char filename[1024];
				FILE *f = popen("zenity --file-selection --directory", "r");
				fgets(filename, 1024, f);
				int r = pclose(f);

				if (r == 0)
				{
					std::string filePath = filename;

					int idx = filePath.find("\n");
					if (idx != std::string::npos)
						filePath = filePath.substr(0, idx);

					filePath = boost::replace_all_copy(filePath, "\\", "/");
					if (filePath.at(filePath.length() - 1) != '/')
						filePath = filePath + "/";

					//Build project
					if (!IO::DirExists(filePath))
						IO::CreateDir(filePath, true);

					BuildSystemWin64::Platform platform = static_cast<BuildSystemWin64::Platform>(currentPlatform);

					MainWindow::addOnEndUpdateCallback([=]()
						{
							auto assets = projectOptimizer->getUsedResources();
							BuildSystemWin64::build(platform, Engine::getSingleton()->getAppName(), filePath, assets);
						}
					);
				}
	#endif
			}
		}

		ImGui::End();

		projectOptimizer->update();
	}
}