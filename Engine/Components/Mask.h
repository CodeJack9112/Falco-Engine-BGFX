#pragma once

#include "UIElement.h"

namespace GX
{
	class Mask : public UIElement
	{
	public:
		Mask();
		virtual ~Mask();

		static std::string COMPONENT_TYPE;
		virtual std::string getComponentType() { return COMPONENT_TYPE; }
		virtual Component* onClone();
		virtual void onRender(ImDrawList* drawList);
	};
}