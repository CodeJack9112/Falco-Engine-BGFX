#pragma once

#include <vector>
#include <string>
#include <thread>

namespace GX
{
	class DialogProgress;

	class BuildSystemWin64
	{
	public:
		enum class Platform
		{
			Windows64,
			Linux64
		};

	private:
		static void packFiles(std::vector<std::string> files, std::string dstName);

		static DialogProgress* progressDialog;

	public:
		BuildSystemWin64();
		~BuildSystemWin64();
		
		static void build(Platform platform, std::string projectName, std::string projectPath, std::vector<std::string> assets);
	};
}