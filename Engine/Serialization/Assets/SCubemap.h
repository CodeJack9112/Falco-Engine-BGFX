#pragma once

#include <string>
#include <vector>

#include "../Serializers/BinarySerializer.h"

namespace GX
{
	class SCubemap : public Archive
	{
	public:
		virtual void serialize(Serializer* s)
		{
			Archive::serialize(s);
			data(textures);
		}

		SCubemap() {}
		~SCubemap() {}

		std::vector<std::string> textures;
	};
}