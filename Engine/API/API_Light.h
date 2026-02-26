#pragma once

#include "API.h"

namespace GX
{
	class API_Light
	{
	public:
		//Register methods
		static void Register()
		{
			mono_add_internal_call("FalcoEngine.Light::get_type", (void*)getType);
			mono_add_internal_call("FalcoEngine.Light::set_type", (void*)setType);

			mono_add_internal_call("FalcoEngine.Light::get_color", (void*)getColor);
			mono_add_internal_call("FalcoEngine.Light::set_color", (void*)setColor);

			mono_add_internal_call("FalcoEngine.Light::get_intensity", (void*)getIntensity);
			mono_add_internal_call("FalcoEngine.Light::set_intensity", (void*)setIntensity);

			mono_add_internal_call("FalcoEngine.Light::get_radius", (void*)getRadius);
			mono_add_internal_call("FalcoEngine.Light::set_radius", (void*)setRadius);

			mono_add_internal_call("FalcoEngine.Light::get_innerRadius", (void*)getInnerRadius);
			mono_add_internal_call("FalcoEngine.Light::set_innerRadius", (void*)setInnerRadius);

			mono_add_internal_call("FalcoEngine.Light::get_outerRadius", (void*)getOuterRadius);
			mono_add_internal_call("FalcoEngine.Light::set_outerRadius", (void*)setOuterRadius);

			mono_add_internal_call("FalcoEngine.Light::get_bias", (void*)getBias);
			mono_add_internal_call("FalcoEngine.Light::set_bias", (void*)setBias);

			mono_add_internal_call("FalcoEngine.Light::get_castShadows", (void*)getCastShadows);
			mono_add_internal_call("FalcoEngine.Light::set_castShadows", (void*)setCastShadows);
		}

	private:
		static int getType(MonoObject* this_ptr);
		static void setType(MonoObject* this_ptr, int value);

		static void getColor(MonoObject* this_ptr, API::Color* out_color);
		static void setColor(MonoObject* this_ptr, API::Color* ref_color);

		static float getIntensity(MonoObject* this_ptr);
		static void setIntensity(MonoObject* this_ptr, float value);
		
		static float getRadius(MonoObject* this_ptr);
		static void setRadius(MonoObject* this_ptr, float value);
		
		static float getInnerRadius(MonoObject* this_ptr);
		static void setInnerRadius(MonoObject* this_ptr, float value);
		
		static float getOuterRadius(MonoObject* this_ptr);
		static void setOuterRadius(MonoObject* this_ptr, float value);
		
		static float getBias(MonoObject* this_ptr);
		static void setBias(MonoObject* this_ptr, float value);

		static bool getCastShadows(MonoObject* this_ptr);
		static void setCastShadows(MonoObject* this_ptr, bool value);
	};
}