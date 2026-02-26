#pragma once

#include <string>
#include <vector>
#include <map>

namespace GX
{
	class TreeView;
	class TreeNode;

	class GameObject;
	class Transform;
	class Texture;

	class HierarchyWindow
	{
	private:
		bool opened = true;
		bool isWindowDragging = false;
		TreeView* treeView = nullptr;
		bool focused = false;

		Texture * addIcon = nullptr;
		Texture * closeIcon = nullptr;

		Texture * exportIcon = nullptr;
		Texture * renameIcon = nullptr;
		Texture * deleteIcon = nullptr;
		Texture * duplicateIcon = nullptr;

		TreeNode* insertNode1(Transform* t, Transform* parent, int index = -1);
		TreeNode* insertNode2(Transform* t, TreeNode* nodeParent, int index = -1);

		void updateTreeNodes(TreeNode* root);

		static std::vector<GameObject*> ignoreObjects;

	public:
		HierarchyWindow();
		~HierarchyWindow();

		void init();
		void update();
		void show(bool show) { opened = show; }
		bool getVisible() { return opened; }

		TreeView* getTreeView() { return treeView; }

		void updateObjectsMenu();
		bool isNodeChildOf(GameObject* node, GameObject* parent);

		static void onNodesSelected(std::vector<std::string> names, void* userData);
		void onDropNode(TreeNode* node, TreeNode* from);
		void onReorder(TreeNode* node, int newIndex);
		void onReorder(std::map<TreeNode*, std::pair<TreeNode*, std::pair<int, int>>> nodes);
		std::vector<bool> onNodesMoved(std::vector<TreeNode*> items, TreeNode* moveto);
		bool onNodeRenamed(TreeNode* item, std::string oldName);
		void onNodePopupMenu(TreeNode* node, int val);

		void refreshHierarchy();
		TreeNode * insertNode(Transform * t, Transform * parent, int index = -1);
		void updateNodeIcon(Transform * t);

		bool isFocused() { return focused; }

		static void setIgnoreObject(GameObject* object, bool value);
	};
}