#pragma once

#include <string>
#include <vector>
#include <functional>

namespace GX
{
	class TreeView;
	class TreeNode;
	class Property;
	class GameObject;

	class PropertyEditor
	{
	public:
		PropertyEditor();
		virtual ~PropertyEditor();

		virtual void update();
		virtual void updateEditor() {}
		
		void setEditorName(std::string name) { editorName = name; }
		std::string getEditorName() { return editorName; }

		void addProperty(Property* prop);
		void removeProperty(Property* prop);
		TreeView* getTreeView() { return treeView; }
		void setParentEditor(PropertyEditor* ed, TreeNode* node) { parentEditor = ed; parentEditorNode = node; }
		PropertyEditor* getParentEditor() { return parentEditor; }
		TreeNode* getParentEditorNode() { return parentEditorNode; }

	protected:
		bool isWindowDragging = false;
		TreeView* treeView = nullptr;
		TreeNode* parentEditorNode = nullptr;
		std::vector<Property*> properties;
		PropertyEditor* parentEditor = nullptr;
		std::string editorName = "PropertyEditor";
		static float previewHeight;

		void setPreviewFunction(std::function<void()> func) { updatePreviewFunc = func; }
		bool isPreviewHovered();

	private:
		float cntSize1 = -1.0f;
		std::function<void()> updatePreviewFunc = nullptr;
	};
}