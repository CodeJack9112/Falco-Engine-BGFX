#pragma once

#include <string>
#include <vector>

namespace GX
{
	class TreeView;
	class TreeNode;
	class DialogProgress;

	class DialogAssetExporter
	{
	public:
		enum class Mode { Import, Export };

		DialogAssetExporter();
		~DialogAssetExporter();

		void show(Mode m);
		void update();

		Mode getMode() { return mode; }
		
	private:
		bool visible = false;
		bool allSelected = true;
		Mode mode = Mode::Export;
		void setMode(Mode m) { mode = m; }

		std::string openedPackage = "";
		TreeView* treeView = nullptr;
		static DialogProgress* progressDialog;

		void listFiles(std::vector<std::string>& outList, TreeNode * htStart);

		void copyTreeView(TreeNode * root, TreeNode * dstRoot);
		void exportAssets(std::string filePath);
		void importAssets();

		void checkNodes(TreeNode * root, bool check);

		static void progressCallback(double progress);
	};
}