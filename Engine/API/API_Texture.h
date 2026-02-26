#pragma once

#include "API.h"

namespace GX
{
	class API_Texture
	{
	public:
		static void Register()
		{
			mono_add_internal_call("FalcoEngine.Texture::INTERNAL_load", (void*)load);
			mono_add_internal_call("FalcoEngine.Texture::get_width", (void*)getWidth);
			mono_add_internal_call("FalcoEngine.Texture::get_height", (void*)getHeight);
			mono_add_internal_call("FalcoEngine.Texture::INTERNAL_fromBytesRGBA8", (void*)fromBytesRGBA8);
		}

	private:
		static MonoObject* load(MonoString* path);
		static int getWidth(MonoObject * this_ptr);
		static int getHeight(MonoObject * this_ptr);
		static MonoObject* fromBytesRGBA8(MonoArray* data, int width, int height);
	};
}