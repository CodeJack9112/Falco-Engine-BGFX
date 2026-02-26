#pragma once

#include <string>

#include "SComponent.h"

namespace GX
{
	class SUIElement : public SComponent
	{
	public:
		SUIElement() = default;
		~SUIElement() {}

		virtual void serialize(Serializer* s)
		{
			SComponent::serialize(s);
			data(horizontalCanvasAlignment);
			data(verticalCanvasAlignment);
			data(anchor);
			data(color);
			data(size);
			data(raycastTarget);
		}

	public:
		int horizontalCanvasAlignment = 0;
		int verticalCanvasAlignment = 0;
		SVector2 anchor = SVector2(0.5, 0.5);
		SVector2 size = SVector2(1.0, 1.0);
		SColor color = SColor(1.0, 1.0, 1.0, 1.0);
		bool raycastTarget = true;
	};
}