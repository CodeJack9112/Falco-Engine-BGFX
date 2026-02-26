#pragma once

#include <string>
#include <dear-imgui/imgui.h>

#include "../Serialization/SProjectData.h"
#include "../Engine/Renderer/Window.h"

namespace GX
{
	class Camera;

	class ProjectSelector : public Window
	{
	private:
		SProjectInfo projectsInfo;

		int openProject(SProject& project);
		int openNewProject();
		int createNewProject(std::string projName, std::string projPath);
		bool isProjectExists(std::string path);
		void loadProjects();
		void saveProjects();

		void onUI();

		ImFont* stdFont = nullptr;
		ImFont* stdBigFont = nullptr;

		int window_w = 0;
		int window_h = 0;

		std::string version = "0.0.0.0";
		int projectToRemove = -1;
		void onProjectRemove(int i);

	public:
		bool start();
	};
}