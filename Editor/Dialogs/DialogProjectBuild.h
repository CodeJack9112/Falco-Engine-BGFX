#pragma once

namespace GX
{
	class ProjectOptimizer;

	class DialogProjectBuild
	{
	private:
		bool visible = false;
		int selScene = -1;
		
	#ifdef _WIN32
		int currentPlatform = 0;
	#else
		int currentPlatform = 1;
	#endif

		ProjectOptimizer* projectOptimizer = nullptr;

	public:
		DialogProjectBuild();
		~DialogProjectBuild();

		void show();
		void hide();
		void update();
	};
}