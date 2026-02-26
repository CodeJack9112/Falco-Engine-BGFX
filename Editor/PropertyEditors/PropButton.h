#pragma once

#include "Property.h"
#include <string>

namespace GX
{
	class PropertyEditor;
	class Texture;

	class PropButton : public Property
	{
	public:
		PropButton(PropertyEditor* ed, std::string name, std::string val = "");
		~PropButton();

		virtual void update(bool opened) override;

		std::string getValue() { return value; }
		void setValue(std::string val);

		Texture * getImage() { return image; }
		void setImage(Texture * val) { image = val; }

		void setOnClickCallback(std::function<void(Property* prop)> callback) { onClickCallback = callback; }

	private:
		std::string value = "";
		Texture* image = nullptr;

		std::function<void(Property* prop)> onClickCallback = nullptr;
	};
}