#pragma once

#include "../Engine/glm/vec2.hpp"

#include "ComponentEditor.h"

namespace GX
{
	class CanvasEditor : public ComponentEditor
	{
	public:
		CanvasEditor();
		~CanvasEditor() {}

		virtual void init(std::vector<Component*> comps);

	private:
		void onChangeRefScreenSize(Property* prop, glm::vec2 value);
		void onChangeScreenMatchSide(Property* prop, float value);
		void onChangeMode(Property* prop, int value);
	};
}