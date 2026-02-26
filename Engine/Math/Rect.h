#pragma once

namespace GX
{
	struct Rect
	{
	public:
		Rect() = default;
		~Rect() = default;

		float left = 0;
		float right = 0;
		float top = 0;
		float bottom = 0;

		Rect(float l, float t, float r, float b)
		{
			left = l;
			top = t;
			right = r;
			bottom = b;
		}

		Rect operator*(float v)
		{
			return Rect(left * v, top * v, right * v, bottom * v);
		}
	};
}