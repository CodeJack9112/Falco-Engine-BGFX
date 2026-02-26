#pragma once

#include <string>

namespace GX
{
	class Hash
	{
	public:
		static size_t getHash(std::string value)
		{
			return std::hash<std::string>{}(value);
		}
	};
}