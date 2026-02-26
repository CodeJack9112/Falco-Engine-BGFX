#pragma once

#include "Property.h"
#include <string>

namespace GX
{
	class PropertyEditor;

	class PropInt : public Property
	{
	public:
		PropInt(PropertyEditor* ed, std::string name, int val = 0);
		~PropInt();

		virtual void update(bool opened) override;
		void setValue(int val);
		int getValue() { return value; }
		void setMinValue(int value) { minVal = value; }
		void setMaxValue(int value) { maxVal = value; }
		void setOnChangeCallback(std::function<void(Property * prop, int val)> callback) { onChangeCallback = callback; }

		bool getIsDraggable() { return isDraggable; }
		void setIsDraggable(bool value) { isDraggable = value; }

		bool getIsSlider() { return isSlider; }
		void setIsSlider(bool value) { isSlider = value; }

	private:
		int value = 0;
		int minVal = 0;
		int maxVal = 0;
		std::string guid1 = "";
		bool isDraggable = true;
		bool isSlider = true;

		std::function<void(Property * prop, int val)> onChangeCallback = nullptr;
	};
}