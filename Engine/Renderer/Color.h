#pragma once

#include <cstdint>

namespace GX
{
	struct Color
	{
	private:
		float val[4] = { 1, 1, 1, 1 };

	public:
		Color()
		{
			val[0] = 1;
			val[1] = 1;
			val[2] = 1;
			val[3] = 1;
		}
		
		Color(float _r, float _g, float _b, float _a)
		{
			val[0] = _r;
			val[1] = _g;
			val[2] = _b;
			val[3] = _a;
		}

		~Color() {}

		static Color White;
		static Color Black;
		static Color Red;
		static Color Green;
		static Color Blue;
		static Color Yellow;

		float r() { return val[0]; };
		float g() { return val[1]; };
		float b() { return val[2]; };
		float a() { return val[3]; };

		float* ptr() { return val; }

		float& operator[] (const int index) { return val[index]; }
		friend bool operator== (const Color& c1, const Color& c2);
		friend bool operator!= (const Color& c1, const Color& c2);

		static uint32_t packABGR(Color value);
		static Color unpackABGR(uint32_t value);
		
		static uint32_t packRGBA(Color value);
		static Color unpackRGBA(uint32_t value);

		static Color lerp(Color a, Color b, float t);
	};
}