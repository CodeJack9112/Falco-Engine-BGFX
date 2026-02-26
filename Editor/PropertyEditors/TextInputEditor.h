#pragma once

#include "UIElementEditor.h"

#include "../Engine/glm/vec4.hpp"

namespace GX
{
	class TextInputEditor : public UIElementEditor
	{
	public:
		TextInputEditor();
		~TextInputEditor() {}

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
		void onChangeTextColor(Property* prop, Color value);
		void onChangePlaceholderColor(Property* prop, Color value);

		void onChangeFontSize(Property* prop, float value);
		void onChangeFontResolution(Property* prop, int value);
		void onChangeLineSpacing(Property* prop, float value);
		void onChangePadding(Property* prop, glm::vec4 value);
		void onChangeHTextAlign(Property* prop, int value);
		void onChangeVTextAlign(Property* prop, int value);
		void onChangeWordWrap(Property* prop, int value);
		void onChangeMultiline(Property* prop, bool value);
		void onChangeText(Property* prop, std::string value);
		void onChangePlaceholder(Property* prop, std::string value);

		void onChangeInteractable(Property* prop, bool value);
		void onChangeImageType(Property* prop, int value);

		void onDropFont(TreeNode* node, TreeNode* from);
		void onClearFont(TreeNode* prop);
	};
}