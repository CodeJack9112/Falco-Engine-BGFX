#pragma once

#include "Property.h"
#include <string>

namespace GX
{
	class PropertyEditor;

	class PropCustom : public Property
	{
	public:
		PropCustom(PropertyEditor* ed, std::string name);
		~PropCustom();

		bool getUseColumns() { return !skipNode; }
		void setUseColumns(bool value) { skipNode = !value; }

		virtual void update(bool opened) override;
		void setOnUpdateCallback(std::function<void(Property* prop)> callback) { onUpdateCallback = callback; }

	private:
		std::string guid1 = "";

		std::function<void(Property* prop)> onUpdateCallback = nullptr;
	};
}