#pragma once

#include "Property.h"
#include <string>

namespace GX
{
	class PropertyEditor;
	class Texture;

	class PropButton2 : public Property
	{
	public:
		PropButton2(PropertyEditor* ed, std::string name, std::string val = "", std::string val2 = "");
		~PropButton2();

		virtual void update(bool opened) override;

		std::string getValue1() { return value1; }
		std::string getValue2() { return value2; }
		void setValue1(std::string val);
		void setValue2(std::string val);

		Texture * getImage1() { return image1; }
		Texture * getImage2() { return image2; }
		void setImage1(Texture * val) { image1 = val; }
		void setImage2(Texture * val) { image2 = val; }

		void setOnClickCallback(std::function<void(Property* prop, int btn)> callback) { onClickCallback = callback; }

	private:
		std::string value1 = "";
		std::string value2 = "";

		std::function<void(Property* prop, int btn)> onClickCallback = nullptr;
		Texture * image1 = nullptr;
		Texture * image2 = nullptr;
	};
}