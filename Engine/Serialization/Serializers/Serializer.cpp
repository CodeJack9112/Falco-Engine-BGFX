#include "Serializer.h"

#include "../../Classes/md5.h"

namespace GX
{
	bool Serializer::is_big_endian(void)
	{
		union {
			uint32_t i;
			char c[4];
		} bint = { 0x01020304 };

		return bint.c[0] == 1;
	}

	void Serializer::checkInputStream(std::istream* s)
	{
		if (!s->good())
			throw std::invalid_argument("Reading from file failed");
	}

	void Serializer::checkOutputStream(std::ostream* s)
	{
		if (s->bad())
			throw std::invalid_argument("Writing to file failed");
	}

	void Serializer::serialize(std::ostream* s, Archive* a, std::string fileFormat)
	{
		operation = Operation::Serialize;
		stream = s;
		s->seekp(std::ios_base::beg);

		//Write header
		std::string header = md5(fileHeader);
		for (int i = 0; i < header.length(); ++i)
			write(header[i], s);

		//Write endianess
		endian_is_big = is_big_endian();
		write(endian_is_big, s);
		
		//Write format
		std::string format = md5(fileFormat);
		write(format, s);

		//Write version
		write(version, s);

		//Write data
		a->serialize(this);
	}

	void Serializer::deserialize(std::istream* s, Archive* a, std::string fileFormat)
	{
		operation = Operation::Deserialize;
		stream = s;
		s->seekg(std::ios_base::beg);

		//Read header
		std::string headerBase = md5(fileHeader);
		std::string header = "";
		header.resize(headerBase.length());
		for (int i = 0; i < headerBase.length(); ++i)
			read(header[i], s);

		if (header != headerBase)
			throw std::invalid_argument("Incompatible file format");

		//Read endianess
		read(endian_is_big, s);

		//Read format
		std::string format = "";
		read(format, s);

		if (!fileFormat.empty())
		{
			std::string formatBase = md5(fileFormat);
			if (format != formatBase)
				throw std::invalid_argument("Incompatible data type");
		}

		//Read version
		int fileVersion = 0;
		read(fileVersion, s);

		if (fileVersion > version)
			throw std::invalid_argument("Incompatible file format. This file was created with newer engine version");

		//Read data
		a->serialize(this);
	}

	void Archive::serialize(Serializer* s)
	{
		serializer = s;

		if (s->getOperation() == Serializer::Operation::Serialize)
			version = getVersion();

		data(version);
	}
}