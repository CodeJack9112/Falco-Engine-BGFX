#pragma once

#include "API.h"

namespace GX
{
	class API_Graphics
	{
	public:
		//Register methods
		static void Register()
		{
			mono_add_internal_call("FalcoEngine.Graphics::INTERNAL_blit(FalcoEngine.Material,FalcoEngine.RenderTexture,bool)", (void*)blitRt);
		}

	private:
		//Blit
		static void blitRt(MonoObject* material, MonoObject* renderTarget, bool copyToBackBuffer);
	};
}