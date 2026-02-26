#pragma once

#include <string>
#include "SUIElement.h"

namespace GX
{
	class SText : public SUIElement
	{
	public:
		SText() = default;
		~SText() {}

		virtual void serialize(Serializer* s)
		{
			SUIElement::serialize(s);
			data(fontPath);
			data(fontSize);
			data(text);
			data(horizontalTextAlignment);
			data(verticalTextAlignment);
			data(lineSpacing);
			data(fontResolution);
			data(wordWrap);
		}

	public:
		std::string fontPath = "";
		float fontSize = 14;
		int fontResolution = 32;
		float lineSpacing = 0;
		std::string text = "";
		int horizontalTextAlignment = 0;
		int verticalTextAlignment = 0;
		int wordWrap = 1;
	};
}