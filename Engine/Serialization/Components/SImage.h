#pragma once

#include <string>
#include "SUIElement.h"

namespace GX
{
	class SImage : public SUIElement
	{
	public:
		SImage() = default;
		~SImage() {}

		virtual void serialize(Serializer* s)
		{
			SUIElement::serialize(s);
			data(texturePath);
			data(imageType);
		}

	public:
		std::string texturePath;
		int imageType = 0;
	};
}