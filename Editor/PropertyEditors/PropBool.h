#pragma once

#include "Property.h"
#include <string>

namespace GX
{
	class PropertyEditor;

	class PropBool : public Property
	{
	public:
		PropBool(PropertyEditor* ed, std::string name, bool val = false);
		~PropBool();

		virtual void update(bool opened) override;
		void setValue(bool val);
		bool getValue() { return value; }
		void setOnChangeCallback(std::function<void(Property * prop, bool val)> callback) { onChangeCallback = callback; }

	private:
		bool value = false;
		std::string guid1 = "";

		std::function<void(Property * prop, bool val)> onChangeCallback = nullptr;
	};
}