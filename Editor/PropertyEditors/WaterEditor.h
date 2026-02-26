#pragma once

#include "ComponentEditor.h"

#include "../Engine/glm/vec2.hpp"
#include "../Engine/glm/vec3.hpp"
#include "../Engine/Renderer/Color.h"

namespace GX
{
	class Property;
	class Texture;
	class Camera;

	class WaterEditor : public ComponentEditor
	{
	public:
		WaterEditor();
		~WaterEditor();

		virtual void init(std::vector<Component*> comps);

	private:
		void onSetSize(Property* prop, int value);
		void onSetReflections(Property* prop, bool value);
		void onSetReflectSkybox(Property* prop, bool value);
		void onSetReflectObjects(Property* prop, bool value);
		void onSetReflectionsDistance(Property* prop, float value);
		void onSetReflectionsQuality(Property* prop, int value);
	};
}