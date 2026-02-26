#pragma once

#include "Property.h"
#include <string>

namespace GX
{
	class PropertyEditor;

	class PropString : public Property
	{
	public:
		PropString(PropertyEditor* ed, std::string name, std::string val = "");
		~PropString();

		virtual void update(bool opened) override;
		void setValue(std::string val);
		std::string getValue() { return value; }
		void setReadonly(bool val) { readOnly = val; }
		void setMultiline(bool val) { multiline = val; }

		void setOnChangeCallback(std::function<void(Property * prop, std::string val)> callback) { onChangeCallback = callback; }

	private:
		std::string value = "";
		std::string guid1 = "";
		bool readOnly = false;
		bool multiline = false;

		std::function<void(Property * prop, std::string val)> onChangeCallback = nullptr;
	};
}