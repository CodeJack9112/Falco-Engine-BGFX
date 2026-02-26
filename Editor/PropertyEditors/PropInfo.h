#pragma once

#include "Property.h"
#include <string>

namespace GX
{
	class Texture;
	class PropertyEditor;

	class PropInfo : public Property
	{
	public:
		PropInfo(PropertyEditor* ed, std::string name, std::string txt);
		~PropInfo();

		bool getUseColumns() { return !skipNode; }
		void setUseColumns(bool value) { skipNode = !value; }

		virtual void update(bool opened) override;

		void setIcon(Texture* value) { icon = value; }

	private:
		std::string guid1 = "";
		std::string text = "";
		Texture* icon = nullptr;
	};
}