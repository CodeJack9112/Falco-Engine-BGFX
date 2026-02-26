#include "ZipHelper.h"

#include "../../LibZip/include/zip.h"

#include "StringConverter.h"

#ifndef _WIN32
#include <cstring>
#endif

namespace GX
{
	bool ZipHelper::isFileInZip(zip_t* zip, std::string path)
	{
		int err;
		struct zip_stat sb;

		int ret = zip_stat(zip, CP_UNI(path).c_str(), 0, &sb);

		return ret > -1;
	}

	char* ZipHelper::readFileFromZip(zip_t* zip, std::string path, int& outBufSize)
	{
		struct zip_file* zf;
		struct zip_stat sb;
		char buf[100];
		int err;
		int i, len;
		long long sum;

		zip_stat(zip, CP_UNI(path).c_str(), ZIP_FL_ENC_UTF_8, &sb);
		zf = zip_fopen(zip, CP_UNI(path).c_str(), ZIP_FL_ENC_UTF_8);

		char* _output = new char[sb.size];

		sum = 0;
		while (sum != sb.size)
		{
			len = zip_fread(zf, buf, 100);
			memcpy(_output + sum, buf, len);
			sum += len;
		}

		outBufSize = sb.size;

		zip_fclose(zf);

		return _output;
	}

	std::vector<std::string> ZipHelper::getAllFilesNamesInZip(zip_t* zip)
	{
		struct zip_stat sb;

		std::vector<std::string> outVec;

		for (int i = 0; i < zip_get_num_entries(zip, 0); ++i)
		{
			zip_stat_index(zip, i, 0, &sb);
			std::string path = CP_SYS(sb.name);
			outVec.push_back(path);
		}

		return outVec;
	}
}