#pragma once

#include <string>
#include <vector>

#include "../Serializers/BinarySerializer.h"

#include "../Data/SRect.h"

namespace GX
{
	class STexture : public Archive
	{
	public:
		virtual void serialize(Serializer* s)
		{
			Archive::serialize(s);
			data(width);
			data(height);
			data(originalWidth);
			data(originalHeight);
			data(bpp);
			data(numMipMaps);
			data(size);
			data(compressionMethod);
			data(compressionQuality);
			data(genMipMaps);
			data(wrapMode);
			data(filterMode);
			data(maxResolution);
			data(border);
			dataVector(pixels);
		}

		STexture() {}
		~STexture() {}

		int width = 0;
		int height = 0;
		int originalWidth = 0;
		int originalHeight = 0;
		int bpp = 0;
		int numMipMaps = 0;
		int size = 0;
		int compressionMethod = 0;
		int compressionQuality = 0;
		bool genMipMaps = true;
		int wrapMode = 0;
		int filterMode = 0;
		int maxResolution = 4096;
		SRect border;
		std::vector<unsigned char> pixels;
	};
}