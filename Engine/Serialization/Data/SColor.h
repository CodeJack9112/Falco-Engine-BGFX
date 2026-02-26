#pragma once

#include "../../Renderer/Color.h"

#include "../Serializers/BinarySerializer.h"

namespace GX
{
	struct SColor : public Archive
	{
	public:
		SColor() {}
		SColor(float _r, float _g, float _b, float _a)
		{
			r = _r;
			g = _g;
			b = _b;
			a = _a;
		}
		SColor(Color col)
		{
			setValue(col);
		}
		~SColor() {}

		virtual void serialize(Serializer* s)
		{
			Archive::serialize(s);
			data(r);
			data(g);
			data(b);
			data(a);
		}

		void setValue(Color other)
		{
			r = other[0];
			g = other[1];
			b = other[2];
			a = other[3];
		}

		Color getValue()
		{
			return Color(r, g, b, a);
		}

	public:
		float r = 0;
		float g = 0;
		float b = 0;
		float a = 0;
	};
}