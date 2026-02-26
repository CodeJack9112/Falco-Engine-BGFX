#pragma once

#include <string>
#include <vector>

struct zip;
typedef struct zip zip_t;

namespace GX
{
	class ZipHelper
	{
	public:
		static bool isFileInZip(zip_t* zip, std::string path);

		static char* readFileFromZip(zip_t* zip, std::string path, int& outBufSize);

		static std::vector<std::string> getAllFilesNamesInZip(zip_t* zip);
	};
}