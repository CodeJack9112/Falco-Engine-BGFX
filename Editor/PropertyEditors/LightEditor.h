#pragma once

#include "ComponentEditor.h"
#include "../Engine/Renderer/Color.h"

namespace GX
{
	class Property;
	class PropFloat;

	class LightEditor : public ComponentEditor
	{
	public:
		LightEditor();
		~LightEditor();

		virtual void init(std::vector<Component*> comps);

		PropFloat* outerRadius = nullptr;
		PropFloat* innerRadius = nullptr;

	private:
		void onChangeType(Property* prop, int value);
		void onChangeRenderMode(Property* prop, int value);
		void onChangeColor(Property* prop, Color value);
		void onChangeRadius(Property* prop, float value);
		void onChangeInnerRadius(Property* prop, float value);
		void onChangeOuterRadius(Property* prop, float value);
		void onChangeIntensity(Property* prop, float value);
		void onChangeBias(Property* prop, float value);
		void onChangeCastShadows(Property* prop, bool value);
	};
}