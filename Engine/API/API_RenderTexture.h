#pragma once

#include "API.h"

namespace GX
{
	class API_RenderTexture
	{
	public:
		//Register methods
		static void Register()
		{
			mono_add_internal_call("FalcoEngine.RenderTexture::INTERNAL_constructor", (void*)ctor);
			mono_add_internal_call("FalcoEngine.RenderTexture::get_colorTexture", (void*)getColorTexture);
			mono_add_internal_call("FalcoEngine.RenderTexture::get_depthTexture", (void*)getDepthTexture);
			mono_add_internal_call("FalcoEngine.RenderTexture::get_width", (void*)getWidth);
			mono_add_internal_call("FalcoEngine.RenderTexture::get_height", (void*)getHeight);
			mono_add_internal_call("FalcoEngine.RenderTexture::Reset", (void*)reset);
			mono_add_internal_call("FalcoEngine.RenderTexture::INTERNAL_dispose", (void*)dispose);
		}

	private:
		//Constructor
		static void ctor(MonoObject* this_ptr, int w, int h, int type);

		//Get color texture
		static MonoObject* getColorTexture(MonoObject* this_ptr);

		//Get depth texture
		static MonoObject* getDepthTexture(MonoObject* this_ptr);

		//Get width
		static int getWidth(MonoObject* this_ptr);

		//Get height
		static int getHeight(MonoObject* this_ptr);

		//Reset
		static void reset(MonoObject* this_ptr, int w, int h);

		//Dispose
		static void dispose(MonoObject* this_ptr);
	};
}