#pragma once

#include "Property.h"
#include <string>

namespace GX
{
	class PropertyEditor;

	class PropFloat : public Property
	{
	public:
		PropFloat(PropertyEditor* ed, std::string name, float val = 0);
		~PropFloat();

		virtual void update(bool opened) override;
		void setValue(float val);
		float getValue() { return value; }
		void setMinValue(float value) { minVal = value; }
		void setMaxValue(float value) { maxVal = value; }
		void setOnChangeCallback(std::function<void(Property * prop, float val)> callback) { onChangeCallback = callback; }

		bool getIsDraggable() { return isDraggable; }
		void setIsDraggable(bool value) { isDraggable = value; }

	private:
		float value = 0;
		float minVal = 0;
		float maxVal = 0;
		std::string guid1 = "";
		bool isDraggable = true;

		std::function<void(Property * prop, float val)> onChangeCallback = nullptr;
	};
}