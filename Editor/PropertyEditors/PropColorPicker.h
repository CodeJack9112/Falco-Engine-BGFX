#pragma once

#include "Property.h"
#include <string>
#include "../Engine/Renderer/Color.h"

namespace GX
{
	class PropertyEditor;

	class PropColorPicker : public Property
	{
	public:
		PropColorPicker(PropertyEditor* ed, std::string name, Color val = Color::White);
		~PropColorPicker();

		virtual void update(bool opened) override;
		void setValue(Color val);
		Color getValue() { return value; }
		void setOnChangeCallback(std::function<void(Property * prop, Color val)> callback) { onChangeCallback = callback; }

	private:
		Color value = Color::White;
		std::string guid1 = "";

		std::function<void(Property * prop, Color val)> onChangeCallback = nullptr;
	};
}