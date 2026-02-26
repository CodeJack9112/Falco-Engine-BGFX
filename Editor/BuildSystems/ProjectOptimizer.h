#pragma once

#include <string>
#include <vector>

namespace GX
{
	class DialogProgress;
	class Material;
	class SMaterial;
	class Texture;

	class ProjectOptimizer
	{
	private:
		DialogProgress* progressDialog = nullptr;

		void addPath(std::string path, std::vector<std::string>& usedAssets);
		void processScene(std::string path, std::vector<std::string>& usedAssets, std::string format);
		void processMaterial(Material* mat, std::vector<std::string>& usedAssets);
		void processMaterial(SMaterial* mat, std::string location, std::vector<std::string>& usedAssets);
		void processTexture(Texture* tex, std::vector<std::string>& usedAssets);

	public:
		ProjectOptimizer();
		~ProjectOptimizer();

		void update();
		std::vector<std::string> getUsedResources();
	};
}