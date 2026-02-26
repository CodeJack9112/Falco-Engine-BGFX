#pragma once

#include "API.h"

namespace GX
{
	class API_Shader
	{
	public:
		//Register methods
		static void Register()
		{
			//mono_add_internal_call("FalcoEngine.Shader::.ctor", (void*)ctor);
			mono_add_internal_call("FalcoEngine.Shader::INTERNAL_load", (void*)load);
		}

	private:
		//Constructor
		//static void ctor(MonoObject* this_ptr);

		//Find
		static MonoObject* load(MonoString* name);
	};
}