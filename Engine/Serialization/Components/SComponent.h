#pragma once

#include <string>

#include "../Serializers/BinarySerializer.h"

namespace GX
{
	class SComponent : public Archive
	{
	public:
		SComponent() {}
		~SComponent() {}

		virtual void serialize(Serializer* s)
		{
			Archive::serialize(s);
			data(enabled);
			data(index);
		}

	public:
		bool enabled = true;
		int index = 0;
	};
}