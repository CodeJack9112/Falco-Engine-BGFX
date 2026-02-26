#pragma once

#include "API.h"

namespace GX
{
	class API_Image
	{
	public:
		static void Register()
		{
			mono_add_internal_call("FalcoEngine.Image::get_texture", (void*)getTexture);
			mono_add_internal_call("FalcoEngine.Image::set_texture", (void*)setTexture);
		}

	private:
		static MonoObject* getTexture(MonoObject* this_ptr);
		static void setTexture(MonoObject* this_ptr, MonoObject* texture);
	};
}