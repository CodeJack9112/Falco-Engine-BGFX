#pragma once

#include "../../Math/Rect.h"

#include "../Serializers/BinarySerializer.h"

namespace GX
{
	struct SRect : public Archive
	{
	public:
		SRect() {}
		SRect(float l, float t, float r, float b)
		{
			left = l;
			top = t;
			right = r;
			bottom = b;
		}
		SRect(Rect rect)
		{
			setValue(rect);
		}
		~SRect() {}

		virtual void serialize(Serializer* s)
		{
			Archive::serialize(s);
			data(left);
			data(right);
			data(top);
			data(bottom);
		}

		void setValue(Rect other)
		{
			left = other.left;
			right = other.right;
			top = other.top;
			bottom = other.bottom;
		}

		Rect getValue()
		{
			return Rect(left, top, right, bottom);
		}

	public:
		float left = 0;
		float right = 0;
		float top = 0;
		float bottom = 0;
	};
}