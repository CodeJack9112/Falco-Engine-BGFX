#include "ProjectSelector.h"

#include <boost/serialization/serialization.hpp>

#include <dear-imgui/imgui_internal.h>
#include <dear-imgui//misc/cpp/imgui_stdlib.h>

#include "../Classes/SolutionWorker.h"

#include "../Engine/Core/Engine.h"
#include "../Engine/Core/Debug.h"
#include "../Engine/Classes/Helpers.h"
#include "../Engine/Classes/IO.h"
#include "../Engine/Assets/Texture.h"
#include "../Engine/UI/ImGUIWidgets.h"

#include "../Engine/Renderer/Renderer.h"
#include "../Engine/Components/Camera.h"
#include "../Engine/Core/GameObject.h"

#ifdef _WIN32
#include <shobjidl_core.h>
#else
#include <pwd.h>
#endif

#include <SDL2/SDL_video.h>

namespace GX
{
	enum ActivePanel { AP_PROJECTS, AP_NEW_PROJECT };
	ActivePanel activePanel = AP_PROJECTS;

	ImVec4 btn1_color = ImVec4(0.1f, 0.1f, 0.1f, 1.0f);
	ImVec4 btn2_color = ImVec4(0.1f, 0.1f, 0.1f, 1.0f);

	std::vector<std::pair<std::string, std::string>> strList;

	int selItem = -1;

	std::string newProjectName = "";
	std::string newProjectPath = "";

	bool ProjectSelector::start()
	{
		Engine::getSingleton()->setIsRuntimeMode(false);

		init("Falco Engine - Select Project", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 465, SDL_WINDOW_SHOWN);

		version = "version " + Helper::getVersion();

		const ImGuiIO& io = ImGui::GetIO(); (void)io;

		ImGuiStyle& style = ImGui::GetStyle();
		static ImGuiStyle ref_saved_style;
		ref_saved_style = style;

		stdFont = io.Fonts->AddFontFromFileTTF(std::string(Helper::ExePath() + "Editor/Fonts/Roboto-Regular.ttf").c_str(), 15.0f, 0, io.Fonts->GetGlyphRangesCyrillic());
		stdBigFont = io.Fonts->AddFontFromFileTTF(std::string(Helper::ExePath() + "Editor/Fonts/Roboto-Regular.ttf").c_str(), 25.0f, 0, io.Fonts->GetGlyphRangesCyrillic());

		loadProjects();

		int i = 0;

		for (auto it = projectsInfo.projects.begin(); it != projectsInfo.projects.end(); ++it, ++i)
		{
			strList.push_back(make_pair(CP_UNI(it->projectName), CP_UNI(it->projectPath)));
		}

		GameObject* dummy = Engine::getSingleton()->createGameObject();
		Camera* camera = new Camera();
		dummy->addComponent(camera);

		Renderer::getSingleton()->setUICallback([=]() { onUI(); });
		bool closedByUser = run();
			
		strList.clear();

		style = ref_saved_style;

		return closedByUser;
	}

	void ProjectSelector::onUI()
	{
		ImGui::GetStyle().WindowBorderSize = 0;
		ImGui::GetStyle().WindowRounding = 0;
		ImGui::GetStyle().WindowPadding = ImVec2(0, 0);
		ImGui::GetStyle().FramePadding = ImVec2(0, 0);
		ImGui::GetStyle().FrameRounding = 0;
		ImGui::GetStyle().GrabRounding = 0;
		
		ImVec4 * colors = ImGui::GetStyle().Colors;
		colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
		colors[ImGuiCol_WindowBg] = ImVec4(0.17f, 0.17f, 0.17f, 1.0f);
		colors[ImGuiCol_FrameBg] = ImVec4(0.13f, 0.13f, 0.13f, 1.0f);
		colors[ImGuiCol_Header] = ImVec4(0.3f, 0.3f, 0.3f, 1.0f);
		colors[ImGuiCol_HeaderHovered] = ImVec4(0.4f, 0.4f, 0.4f, 1.0f);
		colors[ImGuiCol_HeaderActive] = ImVec4(0.35f, 0.35f, 0.35f, 1.0f);
		colors[ImGuiCol_Button] = ImVec4(0.3f, 0.3f, 0.3f, 1.0f);
		colors[ImGuiCol_ButtonHovered] = ImVec4(0.4f, 0.4f, 0.4f, 1.0f);
		colors[ImGuiCol_ButtonActive] = ImVec4(0.35f, 0.35f, 0.35f, 1.0f);

		window_w = getWidth();
		window_h = getHeight();

		/* GUI */
		bool open = true;
		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(window_w, window_h));
		ImGui::Begin("project_selector", &open, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar);

		if (activePanel == AP_PROJECTS)
		{
			ImGui::PushFont(stdBigFont);
			ImGui::SetCursorPos(ImVec2(10.0f, 23.0f));
			ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "PROJECTS");
			ImGui::PopFont();

			ImGui::PushFont(stdFont);

			ImGui::SetCursorPos(ImVec2(window_w - 120, 28.0f));
			ImGui::TextColored(btn1_color, "NEW");
			if (ImGui::IsMouseHoveringRect(ImVec2(window_w - 120, 28.0f), ImVec2(window_w - 90, 43.0f))/* && !dlgOpened*/)
			{
				btn1_color = ImVec4(0.8f, 0.8f, 0.8f, 1.0f);

				if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
				{
					activePanel = AP_NEW_PROJECT;
				}
			}
			else
				btn1_color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);

			ImGui::SetCursorPos(ImVec2(window_w - 60, 28.0f));
			ImGui::TextColored(btn2_color, "OPEN");
			if (ImGui::IsMouseHoveringRect(ImVec2(window_w - 60, 28.0f), ImVec2(window_w - 25, 43.0f))/* && !dlgOpened*/)
			{
				btn2_color = ImVec4(0.8f, 0.8f, 0.8f, 1.0f);

				if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
				{
					if (openNewProject() == 0)
					{
						close();
					}
				}
			}
			else
				btn2_color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);

			ImGui::SetCursorPos(ImVec2(0.0f, 70.0f));
			ImGui::PushItemWidth(window_w);
			if (ImGui::ListBox2("##listbox", &selItem, strList, 19, [=](int index) { onProjectRemove(index); }))
			{
				if (openProject(projectsInfo.projects.at(selItem)) == 0)
				{
					close();
				}
			}
			ImGui::PopItemWidth();

			ImGui::SetCursorPos(ImVec2(window_w - 80, window_h - 26));
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(25, 2));
			if (ImGui::Button("Exit"))
			{
				closeByUser();
			}
			ImGui::PopStyleVar();

			ImGui::PopFont();

			ImGui::SetCursorPos(ImVec2(10, getHeight() - 25));
			ImGui::Text(version.c_str());
		}

		if (activePanel == AP_NEW_PROJECT)
		{
			ImGui::PushFont(stdBigFont);
			ImGui::SetCursorPos(ImVec2(window_w / 2 - 115, 70.0f));
			ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "CREATE NEW PROJECT");
			ImGui::PopFont();

			ImGui::PushFont(stdFont);

			ImGui::SetCursorPos(ImVec2(window_w / 2 - 100, 120.0f));
			ImGui::Text("Project name");
			ImGui::SetCursorPos(ImVec2(window_w / 2 - 100, 140.0f));
			ImGui::SetNextItemWidth(200);
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(3, 3));
			ImGui::InputText("##project_name", &newProjectName);
			ImGui::PopStyleVar();

			ImGui::SetCursorPos(ImVec2(window_w / 2 - 100, 170.0f));
			ImGui::Text("Project location");
			ImGui::SetCursorPos(ImVec2(window_w / 2 - 100, 190.0f));
			ImGui::SetNextItemWidth(170);
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(3, 3));
			ImGui::InputText("##project_path", &newProjectPath);
			ImGui::PopStyleVar();
			ImGui::SetCursorPos(ImVec2(window_w / 2 + 70, 190.0f));
			if (ImGui::Button("...", ImVec2(30, 21)))
			{
	#ifdef _WIN32
				IFileDialog * pfd;
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

									newProjectPath = filePath;
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

					newProjectPath = filePath;
				}
	#endif
			}

			ImGui::SetCursorPos(ImVec2(window_w / 2 - 100, 225.0f));
			if (ImGui::Button("Back", ImVec2(80, 20)))
			{
				activePanel = AP_PROJECTS;
			}

			ImGui::SetCursorPos(ImVec2(window_w / 2 + 20, 225.0f));
			if (ImGui::Button("Create", ImVec2(80, 20)))
			{
				if (createNewProject(CP_SYS(newProjectName), CP_SYS(newProjectPath)) == 0)
				{
					close();
				}
			}

			ImGui::PopFont();
		}

		ImGui::End();
		/* GUI */

		if (projectToRemove > -1)
		{
			projectsInfo.projects.erase(projectsInfo.projects.begin() + projectToRemove);
			saveProjects();

			strList.erase(strList.begin() + projectToRemove);
			projectToRemove = -1;
		}
	}

	void ProjectSelector::onProjectRemove(int i)
	{
		projectToRemove = i;
	}

	int ProjectSelector::openProject(SProject& project)
	{
		if (IO::DirExists(project.projectPath))
		{
			if (IO::DirExists(project.projectPath + "Assets/"))
			{
				// Create temp directory
				IO::CreateDir(project.projectPath + "Temp/");

				// Create settings directory
				IO::CreateDir(project.projectPath + "Settings/");

				// Create solution directory
				IO::CreateDir(project.projectPath + "Project/");

				// Create library directory
				IO::CreateDir(project.projectPath + "Library/");

				Engine::getSingleton()->setAppName(project.projectName);
				Engine::getSingleton()->setRootPath(project.projectPath);
				Engine::getSingleton()->setAssetsPath(project.projectPath + "Assets/");
				Engine::getSingleton()->setLibraryPath(project.projectPath + "Library/");
				Engine::getSingleton()->setSettingsPath(project.projectPath + "Settings/");
				Engine::getSingleton()->setAssemblyPath(project.projectPath + "Project/bin/Debug/");
				Engine::getSingleton()->setBuiltinResourcesPath(Helper::ExePath() + "BuiltinResources/");

				SolutionWorker worker;
				worker.CreateSolution(project.projectPath, project.projectName);

				//Place opened project to the first position
				auto it = std::find(projectsInfo.projects.begin(), projectsInfo.projects.end(), project);
				auto index = std::distance(projectsInfo.projects.begin(), it);

				SProject p = projectsInfo.projects[index];
				projectsInfo.projects.erase(projectsInfo.projects.begin() + index);
				projectsInfo.projects.insert(projectsInfo.projects.begin(), p);

				saveProjects();

				return 0;
			}
			else
			{
	#ifdef _WIN32
				MessageBox(0, "Error opening project. Assets folder not found!", "Error", MB_ICONERROR | MB_OK);
	#else
				FILE *f = popen("zenity --icon-name='dialog-warning' --no-wrap --info --title='Error' --text='Error opening project. Assets folder not found!'", "r");
				pclose(f);	
	#endif
			}
		}
		else
		{
	#ifdef _WIN32
			MessageBox(0, "Error opening project. Project does not exist!", "Error", MB_ICONERROR | MB_OK);
	#else
			FILE *f = popen("zenity --icon-name='dialog-warning' --no-wrap --info --title='Error' --text='Error opening project. Project does not exist!'", "r");
			pclose(f);
	#endif
		}

		return 1;
	}

	int ProjectSelector::openNewProject()
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
							std::string fileName = StringConvert::ws2s(g_name);
							std::string filePath = StringConvert::ws2s(g_path);

							filePath = boost::replace_all_copy(filePath, "\\", "/");
							if (filePath.at(filePath.length() - 1) != '/')
								filePath = filePath + "/";

							if (IO::DirExists(filePath + "Assets/"))
							{
								SProject proj;
								proj.engineVersion = "1.0";
								proj.projectName = fileName;
								proj.projectPath = filePath;

								std::string tmp = (proj.projectPath + "Temp/");
								std::string set = (proj.projectPath + "Settings/");

								// Create temp directory
								IO::CreateDir(tmp);

								// Create settings directory
								IO::CreateDir(set);

								// Create solution directory
								IO::CreateDir(proj.projectPath + "Project/");

								// Create library directory
								IO::CreateDir(proj.projectPath + "Library/");

								Engine::getSingleton()->setAppName(proj.projectName);
								Engine::getSingleton()->setRootPath(filePath);
								Engine::getSingleton()->setAssetsPath(filePath + "Assets/");
								Engine::getSingleton()->setLibraryPath(filePath + "Library/");
								Engine::getSingleton()->setSettingsPath(filePath + "Settings/");
								Engine::getSingleton()->setAssemblyPath(filePath + "Project/bin/Debug/");
								Engine::getSingleton()->setBuiltinResourcesPath(Helper::ExePath() + "BuiltinResources/");

								SolutionWorker worker;
								worker.CreateSolution(proj.projectPath, proj.projectName);

								if (!isProjectExists(filePath))
								{
									//Add new project to list
									projectsInfo.projects.insert(projectsInfo.projects.begin(), proj);
									saveProjects();
								}
								else
								{
									//Place opened project to the first position
									auto it = std::find(projectsInfo.projects.begin(), projectsInfo.projects.end(), proj);
									auto index = std::distance(projectsInfo.projects.begin(), it);

									SProject p = projectsInfo.projects[index];
									projectsInfo.projects.erase(projectsInfo.projects.begin() + index);
									projectsInfo.projects.insert(projectsInfo.projects.begin(), p);

									saveProjects();
								}

								return 0;
							}
							else
							{
								MessageBoxA(0, "Error opening project. Assets folder not found!", "Error", MB_ICONERROR | MB_OK);
							}
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
				
			std::string fileName = IO::GetFileName(filePath);

			filePath = boost::replace_all_copy(filePath, "\\", "/");
			if (filePath.at(filePath.length() - 1) != '/')
				filePath = filePath + "/";

			if (IO::DirExists(filePath + "Assets/"))
			{
				SProject proj;
				proj.engineVersion = "1.0";
				proj.projectName = fileName;
				proj.projectPath = filePath;

				std::string tmp = (proj.projectPath + "Temp/");
				std::string set = (proj.projectPath + "Settings/");

				// Create temp directory
				IO::CreateDir(tmp);

				// Create settings directory
				IO::CreateDir(set);

				// Create solution directory
				IO::CreateDir(proj.projectPath + "Project/");

				// Create library directory
				IO::CreateDir(proj.projectPath + "Library/");

				Engine::getSingleton()->setAppName(proj.projectName);
				Engine::getSingleton()->setRootPath(filePath);
				Engine::getSingleton()->setAssetsPath(filePath + "Assets/");
				Engine::getSingleton()->setLibraryPath(filePath + "Library/");
				Engine::getSingleton()->setSettingsPath(filePath + "Settings/");
				Engine::getSingleton()->setAssemblyPath(filePath + "Project/bin/Debug/");
				Engine::getSingleton()->setBuiltinResourcesPath(Helper::ExePath() + "BuiltinResources/");

				SolutionWorker worker;
				worker.CreateSolution(proj.projectPath, proj.projectName);

				if (!isProjectExists(filePath))
				{
					//Add new project to list
					projectsInfo.projects.insert(projectsInfo.projects.begin(), proj);
					saveProjects();
				}
				else
				{
					//Place opened project to the first position
					auto it = std::find(projectsInfo.projects.begin(), projectsInfo.projects.end(), proj);
					auto index = std::distance(projectsInfo.projects.begin(), it);

					SProject p = projectsInfo.projects[index];
					projectsInfo.projects.erase(projectsInfo.projects.begin() + index);
					projectsInfo.projects.insert(projectsInfo.projects.begin(), p);

					saveProjects();
				}

				return 0;
			}
			else
			{
				FILE *f = popen("zenity --icon-name='dialog-warning' --no-wrap --info --title='Error' --text='Error opening project. Assets folder not found!'", "r");
				pclose(f);
			}
		}
	#endif

		return -1;
	}

	int ProjectSelector::createNewProject(std::string projName, std::string projPath)
	{
		std::string _projPath = projPath;
		if (_projPath[_projPath.size() - 1] != '/' || _projPath[_projPath.size() - 1] != '\\')
			_projPath += "/";

		SProject proj;
		proj.engineVersion = "1.0";
		proj.projectName = projName;
		proj.projectPath = _projPath + projName + "/";

		boost::replace_all(proj.projectPath, "\\", "/");

		while (proj.projectPath.find("//") != std::string::npos)
		{
			boost::replace_all(proj.projectPath, "//", "/");
		}

		bool good = true;

		if (proj.projectName.empty() || proj.projectPath.empty()) good = false;
		if (IO::DirExists(proj.projectPath)) good = false;
		if (IO::DirExists(proj.projectPath + "Assets/")) good = false;
		if (IO::DirExists(proj.projectPath + "Temp/")) good = false;
		if (IO::DirExists(proj.projectPath + "Settings/")) good = false;
		if (IO::DirExists(proj.projectPath + "Project/")) good = false;
		if (IO::DirExists(proj.projectPath + "Library/")) good = false;
			
		if (good)
		{
			//Create project directories
			IO::CreateDir(proj.projectPath);
			IO::CreateDir(proj.projectPath + "Assets/");
			IO::CreateDir(proj.projectPath + "Temp/");
			IO::CreateDir(proj.projectPath + "Settings/");
			IO::CreateDir(proj.projectPath + "Project/");
			IO::CreateDir(proj.projectPath + "Library/");
		}

		if (good)
		{
			if (IO::DirExists(proj.projectPath) && IO::DirExists(proj.projectPath + "Assets/") && IO::DirExists(proj.projectPath + "Temp/"))
			{
				if (!isProjectExists(proj.projectPath))
				{
					//Add new project to list and save it
					projectsInfo.projects.insert(projectsInfo.projects.begin(), proj);
					saveProjects();
				}
				else
				{
					//Place opened project to the first position
					auto it = std::find(projectsInfo.projects.begin(), projectsInfo.projects.end(), proj);
					auto index = std::distance(projectsInfo.projects.begin(), it);

					SProject p = projectsInfo.projects[index];
					projectsInfo.projects.erase(projectsInfo.projects.begin() + index);
					projectsInfo.projects.insert(projectsInfo.projects.begin(), p);

					saveProjects();
				}

				Engine::getSingleton()->setAppName(proj.projectName);
				Engine::getSingleton()->setRootPath(proj.projectPath);
				Engine::getSingleton()->setAssetsPath(proj.projectPath + "Assets/");
				Engine::getSingleton()->setLibraryPath(proj.projectPath + "Library/");
				Engine::getSingleton()->setSettingsPath(proj.projectPath + "Settings/");
				Engine::getSingleton()->setAssemblyPath(proj.projectPath + "Project/bin/Debug/");
				Engine::getSingleton()->setBuiltinResourcesPath(Helper::ExePath() + "BuiltinResources/");

				SolutionWorker worker;
				worker.CreateSolution(proj.projectPath, proj.projectName);

				//Close window
				return 0;
			}
			else
			{
	#ifdef _WIN32
				MessageBox(0, "Error creating project. Create directories failed!", "Error", MB_ICONERROR | MB_OK);
	#else
				FILE *f = popen("zenity --icon-name='dialog-warning' --no-wrap --info --title='Error' --text='Error creating project. Create directories failed!'", "r");
				pclose(f);
	#endif
			}
		}
		else
		{
	#ifdef _WIN32
			MessageBox(0, "Error creating project. Project name or path is not valid!", "Error", MB_ICONERROR | MB_OK);
	#else
			FILE *f = popen("zenity --icon-name='dialog-warning' --no-wrap --info --title='Error' --text='Error creating project. Project name or path is not valid!'", "r");
			pclose(f);
	#endif
		}

		return -1;
	}

	bool ProjectSelector::isProjectExists(std::string path)
	{
		bool exists = false;
		std::vector<SProject>::iterator it;
		for (it = projectsInfo.projects.begin(); it < projectsInfo.projects.end(); ++it)
		{
			if ((*it).projectPath == path)
			{
				exists = true;
				break;
			}
		}

		return exists;
	}

	void ProjectSelector::loadProjects()
	{
		std::ifstream conffile;

	#ifdef _WIN32
		std::string appDataDir = IO::ReplaceBackSlashes(std::string(getenv("APPDATA"))) + "/";
	#else
		struct passwd *pw = getpwuid(getuid());
		const char *homedir = pw->pw_dir;
		std::string appDataDir = std::string(homedir) + "/";
	#endif
		std::string conf_path = appDataDir + "FalcoEngine/Projects.bin";

		conffile.open(conf_path);
		if (conffile)
		{
			conffile.close();

			try
			{
				std::ifstream ofs(conf_path, std::ios::binary);
				BinarySerializer s;
				s.deserialize(&ofs, &projectsInfo, "Project List");
			}
			catch(const std::exception& e)
			{
				Debug::log(std::string("Couldn't load projects list: ") + e.what(), Debug::DbgColorRed);
				std::cerr << "Couldn't load projects list: " << e.what() << '\n';
			}
			
			std::vector<SProject>::iterator it;
			std::vector<SProject> to_remove;

			for (it = projectsInfo.projects.begin(); it < projectsInfo.projects.end(); ++it)
			{
				while (it->projectPath.find("//") != std::string::npos)
				{
					boost::replace_all(it->projectPath, "//", "/");
				}
			}

			//Remove projects which are not exists anymore
			for (it = projectsInfo.projects.begin(); it < projectsInfo.projects.end(); ++it)
			{
				if (!IO::DirExists((*it).projectPath))
				{
					to_remove.push_back(*it);
				}
			}

			auto pred = [&to_remove](const SProject& key) ->bool
			{
				return std::find(to_remove.begin(), to_remove.end(), key) != to_remove.end();
			};

			projectsInfo.projects.erase(std::remove_if(projectsInfo.projects.begin(), projectsInfo.projects.end(), pred), projectsInfo.projects.end());

			if (to_remove.size() > 0)
				saveProjects();
		}
	}

	void ProjectSelector::saveProjects()
	{
	#ifdef _WIN32
		std::string appDataDir = IO::ReplaceBackSlashes(getenv("APPDATA")) + "/";
	#else
		struct passwd *pw = getpwuid(getuid());
		const char *homedir = pw->pw_dir;
		std::string appDataDir = std::string(homedir) + "/";
	#endif
		IO::CreateDir(appDataDir + "FalcoEngine/");
		std::string conf_path = appDataDir + "FalcoEngine/Projects.bin";

		std::ofstream ofs(conf_path, std::ios::binary);
		BinarySerializer s;
		s.serialize(&ofs, &projectsInfo, "Project List");
	}
}