#pragma once

#include "ComponentEditor.h"

namespace GX
{
	class Property;

	class AnimationEditor : public ComponentEditor
	{
	public:
		AnimationEditor();
		~AnimationEditor();

		virtual void init(std::vector<Component*> comps);

	private:
		void onAddAnimationClip(TreeNode* prop);
		void onDropAnimationClip(TreeNode* prop, TreeNode* from);
		void onRemoveAnimationClip(TreeNode* prop, int index);
		void onDropAnimationClipClip(TreeNode* prop, TreeNode* from, int index, Property* endFrameProp);
		void onChangeAnimationClipName(TreeNode* prop, int index, std::string val);
		void onChangeAnimationClipStartFrame(TreeNode* prop, int index, int val);
		void onChangeAnimationClipEndFrame(TreeNode* prop, int index, int val);
		void onChangeAnimationClipSpeed(TreeNode* prop, int index, float val);
		void onChangeAnimationClipLoop(TreeNode* prop, int index, bool val);
	};
}