#pragma once

#include "../Engine/glm/vec2.hpp"
#include "../Engine/Renderer/Color.h"

#include "ComponentEditor.h"

namespace GX
{
	class PropVector2;
	class PropColorPicker;
	class UIElement;

	class UIElementEditor : public ComponentEditor
	{
	public:
		UIElementEditor();
		~UIElementEditor() {}

		virtual void init(std::vector<Component*> comps);

		void updateValues();

	protected:
		PropColorPicker* color = nullptr;

	private:
		PropVector2* size = nullptr;
		PropVector2* anchor = nullptr;

		void onChangeColor(Property* prop, Color value);
		void onChangeSize(Property* prop, glm::vec2 value);
		void onChangeAnchor(Property* prop, glm::vec2 value);
		void onChangeHAlign(Property* prop, int value);
		void onChangeVAlign(Property* prop, int value);
		void onChangeRaycastTarget(Property* prop, bool value);

		static void fitSize(UIElement* element);
		void onFitSize(Property* prop);
	};
}