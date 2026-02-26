#pragma once

#include "API.h"

namespace GX
{
	class API_RenderSettings
	{
	public:
		//Register methods
		static void Register()
		{
			mono_add_internal_call("FalcoEngine.RenderSettings::INTERNAL_get_ambientColor", (void*)getAmbientColor);
			mono_add_internal_call("FalcoEngine.RenderSettings::INTERNAL_set_ambientColor", (void*)setAmbientColor);
		}

	private:
		//Ambient color
		static void getAmbientColor(API::Color* out_color);
		static void setAmbientColor(API::Color* ref_color);
	};
}