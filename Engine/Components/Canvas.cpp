#include "Canvas.h"

#include <algorithm>

#include "../Core/APIManager.h"
#include "../Renderer/Renderer.h"

namespace GX
{
	std::string Canvas::COMPONENT_TYPE = "Canvas";

	Canvas::Canvas() : Component(APIManager::getSingleton()->canvas_class)
	{

	}

	Canvas::~Canvas()
	{
		realScreenSize.clear();
	}

	void Canvas::onAttach()
	{
		Component::onAttach();

		Renderer::getSingleton()->canvases.push_back(this);
	}

	void Canvas::onDetach()
	{
		Component::onDetach();

		std::vector<Canvas*>& canvases = Renderer::getSingleton()->canvases;

		auto it = std::find(canvases.begin(), canvases.end(), this);

		if (it != canvases.end())
			canvases.erase(it);
	}

	std::string Canvas::getComponentType()
	{
		return COMPONENT_TYPE;
	}

	Component* Canvas::onClone()
	{
		Canvas* newComponent = new Canvas();
		newComponent->enabled = enabled;
		newComponent->refScreenSize = refScreenSize;
		newComponent->screenMatchSide = screenMatchSide;
		newComponent->mode = mode;

		return newComponent;
	}

	void Canvas::setRefScreenSize(glm::vec2 value)
	{
		refScreenSize = value;

		for (auto& key : realScreenSize)
			key.second = value;
	}

	glm::vec2 Canvas::getRealScreenSize(void* win)
	{
		if (realScreenSize.find(win) == realScreenSize.end())
			return refScreenSize;
		else
			return realScreenSize[win];
	}

	void Canvas::setRealScreenSize(glm::vec2 value, void* win)
	{
		realScreenSize[win] = value;
	}
}