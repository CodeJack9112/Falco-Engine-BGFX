#pragma once

#include <string>

#include "SComponent.h"

namespace GX
{
	class STerrain : public SComponent
	{
	public:
		STerrain() {}
		~STerrain() {}

		virtual void serialize(Serializer* s)
		{
			SComponent::serialize(s);
			data(filePath);
		}

	public:
		std::string filePath = "";
	};
}