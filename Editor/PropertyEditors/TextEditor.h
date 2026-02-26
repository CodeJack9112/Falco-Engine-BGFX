#pragma once

#include "UIElementEditor.h"

namespace GX
{
	class TextEditor : public UIElementEditor
	{
	public:
		TextEditor();
		~TextEditor() {}

		virtual void init(std::vector<Component*> comps);

	private:
		void onChangeText(Property * prop, std::string value);
		void onChangeFontSize(Property * prop, float value);
		void onChangeFontResolution(Property * prop, int value);
		void onChangeLineSpacing(Property * prop, float value);
		void onChangeHTextAlign(Property* prop, int value);
		void onChangeVTextAlign(Property* prop, int value);
		void onChangeWordWrap(Property* prop, int value);
		void onDropFont(TreeNode* node, TreeNode* from);
		void onClearFont(TreeNode* prop);
	};
}