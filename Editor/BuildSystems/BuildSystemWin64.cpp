#include "../BuildSystems/BuildSystemWin64.h"

#include <boost/algorithm/string.hpp>

#include "../Windows/MainWindow.h"
#include "../Windows/ConsoleWindow.h"
#include "../Dialogs/DialogProgress.h"

#include "../LibZip/include/zip.h"
#include "../Engine/Classes/StringConverter.h"
#include "../Engine/Classes/IO.h"
#include "../Engine/Classes/Helpers.h"
#include "../Engine/Core/Engine.h"
#include "../Engine/Core/APIManager.h"

namespace GX
{
	DialogProgress* BuildSystemWin64::progressDialog = nullptr;

	BuildSystemWin64::BuildSystemWin64()
	{
		
	}

	BuildSystemWin64::~BuildSystemWin64()
	{
		delete progressDialog;
	}

	void BuildSystemWin64::build(Platform platform, std::string projectName, std::string projectPath, std::vector<std::string> assets)
	{
		if (progressDialog == nullptr)
		{
			progressDialog = new DialogProgress();
			progressDialog->setTitle("Building project");
		}

		progressDialog->show();

		ProjectSettings* settings = Engine::getSingleton()->getSettings();

		//Stage 1. Compiling scripts
		progressDialog->setStatusText("Compiling scripts...");
		progressDialog->setProgress(0.25f);

		APIManager::getSingleton()->compile(APIManager::CompileConfiguration::Release);

		//Stage 2. Copy player files
		progressDialog->setStatusText("Copying player...");
		progressDialog->setProgress(0.45f);

		std::vector<std::pair<std::string, std::string>> copyFiles;

		if (platform == Platform::Windows64)
		{
			copyFiles = {
				{ "mono-2.0-sgen.dll", "mono-2.0-sgen.dll" },
				{ "Player.exe", projectName + ".exe" },
				{ "OpenAL32.dll", "OpenAL32.dll" },
				{ "wrap_oal.dll", "wrap_oal.dll" },
				{ "steam_api64.dll", "steam_api64.dll" },
				{ "avcodec-58.dll", "avcodec-58.dll" },
				{ "avformat-58.dll", "avformat-58.dll" },
				{ "avutil-56.dll", "avutil-56.dll" },
				{ "swresample-3.dll", "swresample-3.dll" },
				{ "swscale-5.dll", "swscale-5.dll" }
			};
		}
		else if (platform == Platform::Linux64)
		{
			copyFiles = {
				{ "Player.app", projectName + ".app" },
			};
		}

		for (auto f_it = copyFiles.begin(); f_it < copyFiles.end(); ++f_it)
		{
			if (IO::FileExists(Helper::ExePath() + (*f_it).first))
				IO::FileCopy(Helper::ExePath() + (*f_it).first, projectPath + (*f_it).second);
		}

		if (platform == Platform::Linux64)
		{
			//Copy .so files
			IO::CreateDir(projectPath + "lib/");
			IO::DirCopy(Helper::ExePath() + "lib/", projectPath + "lib/", true);

			//Set permissions to run as application
			boost::filesystem::permissions(projectPath + projectName + ".app", boost::filesystem::perms::owner_all);
		}

		//Stage 3. Copy mono files
		progressDialog->setStatusText("Copying mono...");
		progressDialog->setProgress(0.50f);

		IO::CreateDir(projectPath + "Mono/");
			
		IO::DirCopy(Helper::ExePath() + "Mono/etc/", projectPath + "Mono/etc/", true);
		IO::DirCopy(Helper::ExePath() + "Mono/lib/", projectPath + "Mono/lib/", true);

		//Stage 4. Copy assets
		progressDialog->setStatusText("Packing assets...");
		progressDialog->setProgress(0.70f);

		std::vector<std::string> excludeExts;
		std::vector<std::string> models3d = Engine::getModel3dFileFormats();
		std::vector<std::string> images = Engine::getImageFileFormats();
		std::vector<std::string> audio = Engine::getAudioFileFormats();
		std::vector<std::string> video = Engine::getVideoFileFormats();

		excludeExts.insert(excludeExts.begin(), models3d.begin(), models3d.end());
		excludeExts.insert(excludeExts.begin(), images.begin(), images.end());
		excludeExts.insert(excludeExts.begin(), audio.begin(), audio.end());
		excludeExts.insert(excludeExts.begin(), video.begin(), video.end());
		excludeExts.push_back("cs");
		excludeExts.push_back("scene");
		excludeExts.push_back("material");
		excludeExts.push_back("cubemap");
		excludeExts.push_back("sh");
		excludeExts.push_back("dll");

		std::vector<std::string> fileList;
		std::vector<std::string> shaderIncFiles;
		IO::listFiles(Engine::getSingleton()->getBuiltinResourcesPath(), true, nullptr, [=, &fileList, &shaderIncFiles](std::string dir, std::string fn) -> bool
			{
				std::string ext = IO::GetFileExtension(fn);

				if (std::find(excludeExts.begin(), excludeExts.end(), ext) == excludeExts.end())
				{
					fileList.push_back(dir + fn);
				}

				if (ext == "sh")
					shaderIncFiles.push_back(dir + fn);

				return true;
			}
		);

		std::string dstName = projectPath + "BuiltinResources.resources";
		if (IO::FileExists(dstName))
			IO::FileDelete(dstName);
		packFiles(fileList, dstName);
		fileList.clear();

		progressDialog->setProgress(0.80f);

		IO::listFiles(Engine::getSingleton()->getAssetsPath(), true, nullptr, [=, &fileList, &shaderIncFiles](std::string dir, std::string fn) -> bool
			{
				std::string ext = IO::GetFileExtension(fn);

				if (std::find(excludeExts.begin(), excludeExts.end(), ext) == excludeExts.end())
				{
					fileList.push_back(dir + fn);
				}

				if (ext == "sh")
					shaderIncFiles.push_back(dir + fn);

				return true;
			}
		);

		for (auto& scene : settings->getScenes())
		{
			std::string scenePath = Engine::getSingleton()->getAssetsPath() + scene;
			if (IO::FileExists(scenePath))
				fileList.push_back(scenePath);
		}

		for (auto& asset : assets)
		{
			if (!IO::FileExists(asset))
				continue;

			fileList.push_back(asset);
		}

		dstName = projectPath + "Assets.resources";
		if (IO::FileExists(dstName))
			IO::FileDelete(dstName);
		packFiles(fileList, dstName);
		fileList.clear();

		IO::CreateDir(projectPath + "Shaders");
		for (auto& sh : shaderIncFiles)
		{
			std::string sdir = IO::GetFilePath(sh);
			sdir = IO::Replace(sdir, Engine::getSingleton()->getAssetsPath(), "");
			sdir = IO::Replace(sdir, Engine::getSingleton()->getBuiltinResourcesPath() + "Shaders/", "");
			if (!IO::DirExists(projectPath + "Shaders/" + sdir))
				IO::CreateDir(projectPath + "Shaders/" + sdir);

			IO::FileCopy(sh, projectPath + "Shaders/" + sdir + "/" + IO::GetFileNameWithExt(sh));
		}

		//Stage 5. Copy assemblies
		progressDialog->setStatusText("Copying C# assemblies...");
		progressDialog->setProgress(0.95f);

		IO::FileCopy(Engine::getSingleton()->getRootPath() + "Project/bin/Release/MainAssembly.dll", projectPath + "MainAssembly.dll");
		IO::FileCopy(Helper::ExePath() + "FalcoEngine.dll", projectPath + "FalcoEngine.dll");

		//External libs
		std::vector<std::string> externalDlls;

		IO::listFiles(Engine::getSingleton()->getAssetsPath(), true, nullptr, [=, &externalDlls](std::string d, std::string f) -> bool
			{
				std::string fileName = d + f;
				if (boost::to_lower_copy(IO::GetFileExtension(fileName)) == "dll")
					externalDlls.push_back(fileName);

				return true;
			}
		);

		for (auto& dll : externalDlls)
		{
			std::string newPath = projectPath + IO::GetFileNameWithExt(dll);
			IO::FileCopy(dll, newPath);
		}

		externalDlls.clear();

		//Stage 6. Copy settings
		progressDialog->setStatusText("Copying settings...");
		progressDialog->setProgress(1.0f);

		IO::CreateDir(projectPath + "Settings/");
		IO::FileCopy(Engine::getSingleton()->getSettingsPath() + "Engine.settings", projectPath + "Settings/Engine.settings");

		progressDialog->hide();

		//MessageBoxA(0, "Building done!", "Done", MB_ICONINFORMATION | MB_OK);
		MainWindow::getConsoleWindow()->log("Building done!", LogMessageType::LMT_INFO);
	}

	void BuildSystemWin64::packFiles(std::vector<std::string> files, std::string dstName)
	{
		int* zErr = nullptr;
		zip_t* _zip = zip_open_z(CP_UNI(dstName).c_str(), ZIP_CREATE, zErr);

		for (auto& file : files)
		{
			std::string zipFileName = IO::RemovePart(file, Engine::getSingleton()->getBuiltinResourcesPath());
			zipFileName = IO::RemovePart(zipFileName, Engine::getSingleton()->getAssetsPath());
			zipFileName = IO::RemovePart(zipFileName, Engine::getSingleton()->getLibraryPath());

			zip_source* source = zip_source_file(_zip, CP_UNI(file).c_str(), 0, 0);
			zip_file_add(_zip, CP_UNI(zipFileName).c_str(), source, ZIP_FL_ENC_UTF_8);
		}

		zip_close_z(_zip);
	}
}