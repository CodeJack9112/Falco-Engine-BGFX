#pragma once

#include "ComponentEditor.h"

namespace GX
{
	class Property;

	class AudioListenerEditor : public ComponentEditor
	{
	public:
		AudioListenerEditor();
		~AudioListenerEditor();

		virtual void init(std::vector<Component*> comps);

	private:
		void onChangeVolume(Property* prop, float val);
	};
}