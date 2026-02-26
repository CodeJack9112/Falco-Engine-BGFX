#pragma once

#include <string>
#include "SUIElement.h"

namespace GX
{
	class SButton : public SUIElement
	{
	public:
		SButton() = default;
		~SButton() {}

		virtual void serialize(Serializer* s)
		{
			SUIElement::serialize(s);
			data(texNormalPath);
			data(texHoverPath);
			data(texActivePath);
			data(texDisabledPath);
			data(colHover);
			data(colActive);
			data(colDisabled);
			data(interactable);
			data(imageType);
		}

	public:
		std::string texNormalPath;
		std::string texHoverPath;
		std::string texActivePath;
		std::string texDisabledPath;
		SColor colHover = SColor(1, 1, 1, 1);
		SColor colActive = SColor(1, 1, 1, 1);
		SColor colDisabled = SColor(1, 1, 1, 1);
		bool interactable = true;
		int imageType = 0;
	};
}