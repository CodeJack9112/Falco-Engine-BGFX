#include "API_Screen.h"

#include "../Renderer/Renderer.h"

namespace GX
{
	int API_Screen::getWidth()
	{
		return Renderer::getSingleton()->getWidth();
	}

	int API_Screen::getHeight()
	{
		return Renderer::getSingleton()->getHeight();
	}
}