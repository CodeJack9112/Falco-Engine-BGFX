#include "SolutionWorker.h"
#include "../Engine/Core/Engine.h"

#include "../Engine/Core/APIManager.h"
#include "../Engine/Classes/GUIDGenerator.h"

namespace GX
{
	void SolutionWorker::CreateSolution(std::string dir, std::string projectName, std::vector<std::string> files)
	{
		std::string project_guid = boost::to_upper_copy(GUIDGenerator::genGuid());

		if (IO::FileExists(dir + projectName + ".sln"))
		{
			std::string slnSrc = IO::ReadText(dir + projectName + ".sln");
			boost::regex xRegEx(std::string("Project\\(\\\"\\{([\\w|\\d|-]+)\\}\\\"\\)"));
			boost::smatch xResults;

			if (boost::regex_search(slnSrc, xResults, xRegEx, boost::match_default))
			{
				if (xResults.size() > 1)
					project_guid = xResults[1];
			}
		}

		std::string _namespace = boost::replace_all_copy(projectName, std::string(" "), std::string(""));

		std::string solution = boost::replace_all_copy(solutionText, std::string("%ProjectName%"), projectName);
		solution = boost::replace_all_copy(solution, std::string("%GUID%"), project_guid);
		std::string project = boost::replace_all_copy(projectText, std::string("%ProjectName%"), projectName);
		project = boost::replace_all_copy(project, std::string("%GUID%"), project_guid);

		//Source files
		std::string sources = "";

		for (std::vector<std::string>::iterator it = files.begin(); it != files.end(); ++it)
		{
			sources += "		<Compile Include=\"" + *it + "\" />\n";
		}

		//Dlls
		std::vector<std::string> externalDlls;
		std::string dllsStr = "";

		IO::listFiles(Engine::getSingleton()->getAssetsPath(), true, nullptr, [=, &externalDlls](std::string d, std::string f) -> bool
			{
				std::string fileName = d + f;
				if (boost::to_lower_copy(IO::GetFileExtension(fileName)) == "dll")
					externalDlls.push_back(fileName);

				return true;
			}
		);

		int i = 0;
		for (auto dll = externalDlls.begin(); dll != externalDlls.end(); ++dll, ++i)
		{
			std::string dllName = IO::GetFileName(*dll);

			dllsStr += "		<Reference Include = \"" + dllName + "\">\n";
			dllsStr += "			<HintPath>" + *dll + "</HintPath>\n";
			dllsStr += "		</Reference>";

			if (i < externalDlls.size() - 1)
				dllsStr += "\n";
		}

		externalDlls.clear();

		project = boost::replace_all_copy(project, std::string("%FalcoEngineDll%"), Helper::ExePath() + "FalcoEngine.dll");
		project = boost::replace_all_copy(project, std::string("%SourceFiles%"), sources);
		project = boost::replace_all_copy(project, std::string("%ExternalDlls%"), dllsStr);

		project = CP_UNI(project.c_str());

		solution = CP_UNI(solution);

		if (!IO::FileExists(dir + projectName + ".sln"))
			IO::WriteText(dir + projectName + ".sln", solution);

		IO::WriteText(dir + projectName + ".csproj", project);

		if (!IO::FileExists(dir + "Project/bin/Debug/MainAssembly.dll"))
		{
			APIManager::getSingleton()->compile(APIManager::CompileConfiguration::Debug);
		}
	}
}