#include "Mask.h"

#include <imgui.h>
#include <imgui_internal.h>

#include "../Core/APIManager.h"

namespace GX
{
	std::string Mask::COMPONENT_TYPE = "Mask";

	Mask::Mask() : UIElement(APIManager::getSingleton()->mask_class)
	{
	}

	Mask::~Mask()
	{
	}

	Component* Mask::onClone()
	{
		Mask* newComponent = new Mask();
		newComponent->enabled = enabled;
		newComponent->anchor = anchor;
		newComponent->color = color;
		newComponent->size = size;
		newComponent->horizontalAlignment = horizontalAlignment;
		newComponent->verticalAlignment = verticalAlignment;
		newComponent->raycastTarget = raycastTarget;

		return newComponent;
	}

	void Mask::onRender(ImDrawList* drawList)
	{
		UIElement::onRender(drawList);
	}
}