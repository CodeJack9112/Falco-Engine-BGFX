#pragma once

#include "UIElementEditor.h"

namespace GX
{
	class ImageEditor : public UIElementEditor
	{
	public:
		ImageEditor();
		~ImageEditor() {}

		virtual void init(std::vector<Component*> comps);

	private:
		void onChangeTexture(TreeNode* prop, TreeNode* from);
		void onClearTexture(TreeNode* prop);
		void onChangeImageType(Property* prop, int value);
	};
}