#pragma once

#include <string>
#include "SUIElement.h"

namespace GX
{
	class STextInput : public SUIElement
	{
	public:
		STextInput() = default;
		~STextInput() {}

		virtual void serialize(Serializer* s)
		{
			SUIElement::serialize(s);
			data(fontPath);
			data(texNormalPath);
			data(texHoverPath);
			data(texActivePath);
			data(texDisabledPath);
			data(colHover);
			data(colActive);
			data(colDisabled);
			data(colText);
			data(colPlaceholder);
			data(imageType);
			data(text);
			data(placeholder);
			data(multiline);
			data(fontSize);
			data(fontResolution);
			data(lineSpacing);
			data(padding);
			data(horizontalTextAlignment);
			data(verticalTextAlignment);
			data(wordWrap);
			data(interactable);
		}

	public:
		std::string fontPath = "";
		std::string texNormalPath;
		std::string texHoverPath;
		std::string texActivePath;
		std::string texDisabledPath;
		SColor colHover = SColor(1, 1, 1, 1);
		SColor colActive = SColor(1, 1, 1, 1);
		SColor colDisabled = SColor(1, 1, 1, 1);
		SColor colText = SColor(1, 1, 1, 1);
		SColor colPlaceholder = SColor(1, 1, 1, 1);
		int imageType = 0;
		std::string text = "";
		std::string placeholder = "";
		bool multiline = false;
		float fontSize = 14.0f;
		int fontResolution = 32;
		float lineSpacing = 0;
		SVector4 padding = SVector4(5.0f, 5.0f, 5.0f, 5.0f);
		int horizontalTextAlignment = 0;
		int verticalTextAlignment = 0;
		int wordWrap = 1;
		bool interactable = true;
	};
}