#pragma once

#include "Property.h"
#include <string>

namespace GX
{
	class PropertyEditor;
	class Asset;
	class Texture;

	class PropAsset : public Property
	{
	public:
		PropAsset(PropertyEditor* ed, std::string name, Asset* val = nullptr);
		~PropAsset();

		virtual void update(bool opened) override;
		void setValue(Asset* val);
		Asset* getValue() { return value; }

		Texture* getImage() { return customImage; }
		void setImage(Texture* val) { customImage = val; }

		void setOnClickCallback(std::function<void(Property* prop)> callback) { onClickCallback = callback; }
		void setOnChangeCallback(std::function<void(Property * prop, Asset* val)> callback) { onChangeCallback = callback; }

	private:
		Asset* value = nullptr;
		std::string guid1 = "";
		std::string fileName = "[None]";
		Texture* image = nullptr;
		Texture* customImage = nullptr;

		std::function<void(Property* prop)> onClickCallback = nullptr;
		std::function<void(Property* prop, Asset* val)> onChangeCallback = nullptr;
	};
}