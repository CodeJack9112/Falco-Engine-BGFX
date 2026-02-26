#pragma once

#include <string>
#include "SUIElement.h"

namespace GX
{
	class SMask : public SUIElement
	{
	public:
		SMask() = default;
		~SMask() {}

		virtual void serialize(Serializer* s)
		{
			SUIElement::serialize(s);
		}
	};
}