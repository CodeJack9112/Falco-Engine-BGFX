#pragma once

#include "Property.h"
#include <string>

namespace GX
{
	class PropertyEditor;
	class Texture;

	class PropSampler : public Property
	{
	public:
		PropSampler(PropertyEditor* ed, std::string name, Texture * val = nullptr);
		~PropSampler();

		virtual void update(bool opened) override;
		Texture* getValue() { return value; }
		void setValue(Texture* val);

		void setOnClickCallback(std::function<void(Property* prop)> callback) { onClickCallback = callback; }

	private:
		std::function<void(Property* prop)> onClickCallback = nullptr;
		Texture * value = nullptr;
	};
}