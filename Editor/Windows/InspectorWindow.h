#pragma once

#include <vector>

namespace GX
{
	class PropertyEditor;
	class TreeNode;

	class InspectorWindow
	{
	private:
		bool opened = true;
		PropertyEditor* currentEditor = nullptr;
		float scrollPos = 0;
		float setScrollPosValue = -1;
		bool focused = false;

		void getAllProperties(TreeNode* root, std::vector<TreeNode*>& list);

	public:
		InspectorWindow();
		~InspectorWindow();

		void init();
		void update();
		void show(bool show) { opened = show; }
		bool getVisible() { return opened; }

		void setEditor(PropertyEditor* editor);
		PropertyEditor* getCurrentEditor() { return currentEditor; }
		void updateCurrentEditor();
		void updateObjectEditorTransform();
		float getScrollPos() { return scrollPos; }
		void setScrollPos(float value) { setScrollPosValue = value; }
		void saveCollapsedProperties();
		void loadCollapsedProperties();

		bool isFocused() { return focused; }
	};
}