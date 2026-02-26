#include "API_RenderSettings.h"

#include "../Renderer/Renderer.h"

namespace GX
{
	void API_RenderSettings::getAmbientColor(API::Color* out_color)
	{
		Color color = Renderer::getSingleton()->getAmbientColor();

		out_color->r = color[0];
		out_color->g = color[1];
		out_color->b = color[2];
		out_color->a = color[3];
	}

	void API_RenderSettings::setAmbientColor(API::Color* ref_color)
	{
		Color& color = Renderer::getSingleton()->getAmbientColor();

		color[0] = ref_color->r;
		color[1] = ref_color->g;
		color[2] = ref_color->b;
		color[3] = ref_color->a;
	}
}