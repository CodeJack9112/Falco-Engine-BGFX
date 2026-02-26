#pragma once

#include <string>
#include <vector>
#include <map>
#include <functional>

namespace GX
{
	class TreeNode;

	class TreeView
	{
	public:
		TreeView();
		~TreeView();

		enum SearchCond { SC_NAME, SC_ALIAS };

	public:
		TreeNode* getNodeByName(std::string name, TreeNode* root, bool recursive = true);
		TreeNode* getNodeByName(std::string name, bool recursive = true);
		TreeNode* getNodeByAlias(std::string alias, TreeNode* root, bool recursive = true);
		TreeNode* getNodeByPath(std::string path, TreeNode* root);
		TreeNode* getRootNode() { return rootNode; }
		static TreeNode* getLastClickedNode() { return lastClickedNode; }
		std::string getNodePath(TreeNode* node);

		void deleteNode(TreeNode * node);
		void expandParent(TreeNode* node);
		void update();
		void clear();
		void selectNodes(std::vector<std::string> names, bool callback = true, void* userData = nullptr);
		void setSelectCallback(void(*callback)(std::vector<std::string> names, void* userData));
		void setOnDropCallback(std::function<void(TreeNode * from, TreeNode * to)> callback) { dropCallback = callback; }
		void setOnReorderCallback(std::function<void(TreeNode * node, int newIndex)> callback) { reorderCallback = callback; }
		void setOnReorderCallback(std::function<void(std::map<TreeNode*, std::pair<TreeNode*, std::pair<int, int>>> nodes)> callback) { reorderCallback2 = callback; }
		void setOnBeginDragCallback(std::function<void(TreeNode * node)> callback) { beginDragCallback = callback; }
		void setOnEndUpdateCallback(std::function<void()> callback) { endUpdateCallback = callback; }
		void setAllowMoveNodes(bool allow) { allowMoveNodes = allow; }
		bool getAllowMoveNodes() { return allowMoveNodes; }
		void updateDragDropTarget();
		void setProcessDragDropInTarget(bool value) { processDragDropInTarget = value; }
		bool getProcessDragDropInTarget() { return processDragDropInTarget; }
		std::string getTag() { return tag; }
		void setTag(std::string value) { tag = value; }
		void setAllowMultipleSelection(bool val) { allowMultipleSelection = val; }
		bool getAllowMultipleSelection() { return allowMultipleSelection; }
		void setAllowReorder(bool value) { allowReorder = value; }
		bool getAllowReorder() { return allowReorder; }
		bool getIsReorderingInProgress() { return isReorderingInProgress; }

		std::vector<TreeNode*>& getSelectedNodes() { return selectedNodes; }
		std::vector<TreeNode*>& getLastSelectedNodes() { return lastSelectedNodes; }
		std::map<std::string, bool> getItemsExpandStates();
		void setItemsExpandStates(std::map<std::string, bool> states);
		void setFilter(std::string value);
		std::string& getFilter() { return filter; }

		void setUseColumns(bool value) { useColumns = value; }
		bool getUseColumns() { return useColumns; }

		void focusOnNode(TreeNode* node) { focusNode = node; }

		bool getHasTopPadding() { return hasTopPadding; }
		void setHasTopPadding(bool value) { hasTopPadding = value; }

	private:
		void updateRecursive(TreeNode* root, bool & wasClicked);
		void getNodeByCondRecursive(std::string search, TreeNode* root, TreeNode*& outNode, SearchCond cond = SearchCond::SC_NAME, bool recursive = true);
		void getItemsExpandStatesRecursive(TreeNode * root, std::map<std::string, bool> & outMap);

		void checkNodeChildren(TreeNode * root, bool check);
		void checkNodeParent(TreeNode * root, bool check);
		void moveNodes(TreeNode * moveto, TreeNode * movefrom, bool useSelected = true);
		void onDragStarted(TreeNode * node);
		void onDragEnded();
		void processDragDrop();
		void updateSelection(bool wasClicked);

		bool useColumns = false;
		TreeNode* editNode = nullptr;
		TreeNode* move_from = nullptr;
		TreeNode* move_to = nullptr;
		TreeNode* shiftSelectNode = nullptr;
		TreeNode* reorderNode = nullptr;
		TreeNode* focusNode = nullptr;

		bool wasClicked = false;
		bool hasTopPadding = false;

		bool dragStarted = false;
		bool resetLastNodes = false;
		TreeNode* separatorOnTopNode = nullptr;

		bool isWindowDragging = false;
		bool allowMoveNodes = false;
		bool processDragDropInTarget = false;
		bool allowMultipleSelection = true;
		bool allowReorder = false;
		bool tempDisableDragDrop = false;
		bool filterIncludeParents = false;
		bool isReorderingInProgress = false;

		float clickTime = 0.0f;
		std::string filter = "";
		std::vector<TreeNode*> filteredNodes;

		std::vector<TreeNode*> selectedNodes;
		std::vector<TreeNode*> lastSelectedNodes;
		TreeNode* rootNode = nullptr;
		static TreeNode* lastClickedNode;
		std::string tag = "";

		void(*selectCallback)(std::vector<std::string> names, void* userData) = nullptr;
		std::function<void(TreeNode* from, TreeNode * to)> dropCallback = nullptr;
		std::function<void(TreeNode* node, int newIndex)> reorderCallback = nullptr;
		std::function<void(std::map<TreeNode*, std::pair<TreeNode*, std::pair<int, int>>> nodes)> reorderCallback2 = nullptr;
		std::function<void(TreeNode* node)> beginDragCallback = nullptr;
		std::function<void()> endUpdateCallback = nullptr;
	};
}