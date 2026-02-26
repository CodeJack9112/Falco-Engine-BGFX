#pragma once

#include "ComponentEditor.h"

namespace GX
{
	class Property;
	class TreeNode;

	class AudioSourceEditor : public ComponentEditor
	{
	public:
		AudioSourceEditor();
		~AudioSourceEditor();

		virtual void init(std::vector<Component*> comps);

	private:
		void onChangePlayOnStart(Property* prop, bool val);
		void onChangeLoop(Property* prop, bool val);
		void onChangeVolume(Property* prop, float val);
		void onChangeMinDistance(Property* prop, float val);
		void onChangeMaxDistance(Property* prop, float val);
		void onChangeIs2D(Property* prop, bool val);
		void onChangePitch(Property* prop, float val);

		void onDropAudioClip(TreeNode * prop, TreeNode * from);
		void onClickAudioClip(Property * prop);
	};
}