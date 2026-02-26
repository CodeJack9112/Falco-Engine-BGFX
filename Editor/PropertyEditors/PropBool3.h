#pragma once

#include "Property.h"
#include <string>

namespace GX
{
	class PropertyEditor;

	class PropBool3 : public Property
	{
	public:
		PropBool3(PropertyEditor* ed, std::string name, std::string lbl1, std::string lbl2, std::string lbl3, bool* val = nullptr);
		~PropBool3();

		virtual void update(bool opened) override;
		void setValue(bool val[3]);
		bool getValue() { return value; }
		void setOnChangeCallback(std::function<void(Property * prop, bool val[3])> callback) { onChangeCallback = callback; }

	private:
		bool value[3] = { false };
		std::string guid1 = "";
		std::string guid2 = "";
		std::string guid3 = "";

		std::string label1 = "X";
		std::string label2 = "Y";
		std::string label3 = "Z";

		std::function<void(Property * prop, bool val[3])> onChangeCallback = nullptr;
	};
}