#pragma once

#include "UIElementEditor.h"

namespace GX
{
	class ButtonEditor : public UIElementEditor
	{
	public:
		ButtonEditor();
		~ButtonEditor() {}

		virtual void init(std::vector<Component*> comps);

	private:
		void onChangeNormalTexture(TreeNode* prop, TreeNode* from);
		void onClearNormalTexture(TreeNode* prop);
		void onChangeHoverTexture(TreeNode* prop, TreeNode* from);
		void onClearHoverTexture(TreeNode* prop);
		void onChangeActiveTexture(TreeNode* prop, TreeNode* from);
		void onClearActiveTexture(TreeNode* prop);
		void onChangeDisabledTexture(TreeNode* prop, TreeNode* from);
		void onClearDisabledTexture(TreeNode* prop);

		void onChangeHoverColor(Property* prop, Color value);
		void onChangeActiveColor(Property* prop, Color value);
		void onChangeDisabledColor(Property* prop, Color value);

		void onChangeInteractable(Property* prop, bool value);
		void onChangeImageType(Property* prop, int value);
	};
}