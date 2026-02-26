#pragma once

#include "ComponentEditor.h"

namespace GX
{
	class Property;
	class TreeNode;

	class VideoPlayerEditor : public ComponentEditor
	{
	public:
		VideoPlayerEditor();
		virtual ~VideoPlayerEditor();

		virtual void init(std::vector<Component*> comps);

	private:
		void onChangePlayOnStart(Property* prop, bool val);
		void onChangeLoop(Property* prop, bool val);

		void onDropVideoClip(TreeNode* prop, TreeNode* from);
		void onClickVideoClip(Property* prop);
	};
}