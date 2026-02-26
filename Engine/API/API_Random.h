#pragma once

#include "API.h"

namespace GX
{
	class API_Random
	{
	public:
		//Register methods
		static void Register()
		{
			mono_add_internal_call("FalcoEngine.Random::RangeFloat", (void*)rangeFloat);
			mono_add_internal_call("FalcoEngine.Random::RangeInt", (void*)rangeInt);
		}

	private:
		//Get position
		static float rangeFloat(float _min, float _max);
		static int rangeInt(int _min, int _max);
	};
}