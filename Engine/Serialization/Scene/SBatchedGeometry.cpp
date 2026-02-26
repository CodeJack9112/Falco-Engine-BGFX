#include "SBatchedGeometry.h"

#include <fstream>
#include <iostream>

#include <boost/iostreams/stream.hpp>

#include "../../Classes/IO.h"
#include "../../Classes/ZipHelper.h"
#include "../../Core/Engine.h"
#include "../../Core/Debug.h"

namespace GX
{
	void SBatch::clear()
	{
		vertexBuffer.clear();
		indexBuffer.clear();
	}

	void SBatchedGeometry::clear()
	{
		for (int i = 0; i < batches.size(); ++i)
			batches[i].clear();

		batches.clear();
	}

	void SBatchedGeometry::save(std::string location, std::string name)
	{
		std::string fullPath = location + name;
		std::ofstream ofs(fullPath, std::ios::binary);
		BinarySerializer s;
		s.serialize(&ofs, this, "Batched Geometry");
		ofs.close();
	}

	bool SBatchedGeometry::load(std::string location, std::string name)
	{
		clear();

		if (IO::isDir(location))
		{
			std::string fullPath = location + name;
			if (!IO::FileExists(fullPath))
			{
				return false;
			}

			try
			{
				std::ifstream ofs(fullPath, std::ios::binary);
				BinarySerializer s;
				s.deserialize(&ofs, this, "Batched Geometry");
				ofs.close();
			}
			catch (std::exception e)
			{
				Debug::log(std::string("Error loading batched geometry: ") + e.what(), Debug::DbgColorRed);
			}
		}
		else
		{
			zip_t* arch = Engine::getSingleton()->getZipArchive(location);
			if (!ZipHelper::isFileInZip(arch, name))
			{
				return false;
			}

			int sz = 0;
			char* buffer = ZipHelper::readFileFromZip(arch, name, sz);

			try
			{
				boost::iostreams::stream<boost::iostreams::array_source> is(buffer, sz);
				BinarySerializer s;
				s.deserialize(&is, this, "Batched Geometry");
				is.close();
			}
			catch (std::exception e)
			{
				Debug::log(std::string("Error loading batched geometry: ") + e.what(), Debug::DbgColorRed);
			}

			delete[] buffer;
		}

		return true;
	}
}