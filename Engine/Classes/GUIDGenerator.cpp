#include "GUIDGenerator.h"

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/algorithm/string.hpp>

namespace GX
{
	std::string GUIDGenerator::genGuid()
	{
		boost::uuids::random_generator uuid_gen;
		boost::uuids::uuid u = uuid_gen();

		return boost::uuids::to_string(u);
	}
}