#include "BinarySerializer.h"

#ifndef _WIN32
#include <cstring>
#endif

#include "../Engine/Core/Debug.h"

namespace GX
{
	//Write

	void BinarySerializer::writeInt(int& t, std::ostream* s)
	{
		char d[4];

		std::copy(static_cast<const char*>(static_cast<const void*>(&t)),
			static_cast<const char*>(static_cast<const void*>(&t)) + (sizeof(char) * 4),
			d);

		s->write(reinterpret_cast<const char*>(d), sizeof(char) * 4);
	}

	void BinarySerializer::writeUInt(unsigned int& t, std::ostream* s)
	{
		char d[4];

		std::copy(static_cast<const char*>(static_cast<const void*>(&t)),
			static_cast<const char*>(static_cast<const void*>(&t)) + (sizeof(char) * 4),
			d);

		s->write(reinterpret_cast<const char*>(d), sizeof(char) * 4);
	}

	void BinarySerializer::writeFloat(float& t, std::ostream* s)
	{
		char d[4];

		std::copy(static_cast<const char*>(static_cast<const void*>(&t)),
			static_cast<const char*>(static_cast<const void*>(&t)) + (sizeof(char) * 4),
			d);

		s->write(reinterpret_cast<const char*>(d), sizeof(char) * 4);
	}

	void BinarySerializer::writeDouble(double& t, std::ostream* s)
	{
		char d[8];

		std::copy(static_cast<const char*>(static_cast<const void*>(&t)),
			static_cast<const char*>(static_cast<const void*>(&t)) + (sizeof(char) * 8),
			d);

		s->write(reinterpret_cast<const char*>(d), sizeof(char) * 8);
	}

	void BinarySerializer::writeBool(bool& t, std::ostream* s)
	{
		char byte = t ? (char)1 : (char)0;
		s->write(reinterpret_cast<const char*>(&byte), sizeof(char));
	}

	void BinarySerializer::writeChar(char& t, std::ostream* s)
	{
		s->write(reinterpret_cast<const char*>(&t), sizeof(char));
	}

	void BinarySerializer::writeUChar(unsigned char& t, std::ostream* s)
	{
		char c = static_cast<char>(t);
		s->write(reinterpret_cast<const char*>(&c), sizeof(char));
	}

	void BinarySerializer::writeString(std::string& t, std::ostream* s)
	{
		int len = static_cast<int>(t.length());
		write(len, s);
		s->write(t.c_str(), sizeof(char) * len);
	}

	void BinarySerializer::writeULong(unsigned long long& t, std::ostream* s)
	{
		char d[8];

		std::copy(static_cast<const char*>(static_cast<const void*>(&t)),
			static_cast<const char*>(static_cast<const void*>(&t)) + (sizeof(char) * 8),
			d);

		s->write(reinterpret_cast<const char*>(d), sizeof(char) * 8);
	}

	void BinarySerializer::writeVectorInt(std::vector<int>& t, std::ostream* s)
	{
		int size = (int)t.size();
		write(size, s);

		char* buf = new char[4 * size];

		std::copy(static_cast<const char*>(static_cast<const void*>(&t[0])),
			static_cast<const char*>(static_cast<const void*>(&t[0])) + (sizeof(char) * 4 * size),
			buf);

		s->write(reinterpret_cast<const char*>(buf), sizeof(char) * 4 * size);

		delete[] buf;
	}

	void BinarySerializer::writeVectorUInt(std::vector<unsigned int>& t, std::ostream* s)
	{
		int size = (int)t.size();
		write(size, s);

		char* buf = new char[4 * size];

		std::copy(static_cast<const char*>(static_cast<const void*>(&t[0])),
			static_cast<const char*>(static_cast<const void*>(&t[0])) + (sizeof(char) * 4 * size),
			buf);

		s->write(reinterpret_cast<const char*>(buf), sizeof(char) * 4 * size);

		delete[] buf;
	}

	void BinarySerializer::writeVectorFloat(std::vector<float>& t, std::ostream* s)
	{
		int size = (int)t.size();
		write(size, s);

		char* buf = new char[4 * size];

		std::copy(static_cast<const char*>(static_cast<const void*>(&t[0])),
			static_cast<const char*>(static_cast<const void*>(&t[0])) + (sizeof(char) * 4 * size),
			buf);

		s->write(reinterpret_cast<const char*>(buf), sizeof(char) * 4 * size);

		delete[] buf;
	}

	void BinarySerializer::writeVectorUChar(std::vector<unsigned char>& t, std::ostream* s)
	{
		int size = (int)t.size();
		write(size, s);

		char* buf = new char[size];
		memcpy(buf, &t[0], size * sizeof(char));

		s->write(buf, size * sizeof(char));

		delete[] buf;
	}

	void BinarySerializer::writeVectorULong(std::vector<unsigned long long>& t, std::ostream* s)
	{
		int size = (int)t.size();
		write(size, s);

		char* buf = new char[8 * size];

		std::copy(static_cast<const char*>(static_cast<const void*>(&t[0])),
			static_cast<const char*>(static_cast<const void*>(&t[0])) + (sizeof(char) * 8 * size),
			buf);

		s->write(reinterpret_cast<const char*>(buf), sizeof(char) * 8 * size);

		delete[] buf;
	}

	//Read

	void BinarySerializer::readInt(int& t, std::istream* s)
	{
		char d[4];
		s->read(static_cast<char*>(d), sizeof(char) * 4);

		char b[4];

		if (is_big_endian() == endian_is_big)
		{
			for (int i = 0; i < 4; ++i)
				b[i] = d[i];
		}
		else
		{
			for (int i = 0; i < 4; ++i)
				b[i] = d[3 - i];
		}

		std::copy(static_cast<const char*>(static_cast<const void*>(b)),
			static_cast<const char*>(static_cast<const void*>(b)) + (sizeof(char) * 4),
			static_cast<char*>(static_cast<void*>(&t)));
	}

	void BinarySerializer::readUInt(unsigned int& t, std::istream* s)
	{
		char d[4];
		s->read(d, sizeof(char) * 4);

		char b[4];

		if (is_big_endian() == endian_is_big)
		{
			for (int i = 0; i < 4; ++i)
				b[i] = d[i];
		}
		else
		{
			for (int i = 0; i < 4; ++i)
				b[i] = d[3 - i];
		}

		std::copy(static_cast<const char*>(static_cast<const void*>(b)),
			static_cast<const char*>(static_cast<const void*>(b)) + (sizeof(char) * 4),
			static_cast<char*>(static_cast<void*>(&t)));
	}

	void BinarySerializer::readFloat(float& t, std::istream* s)
	{
		char d[4];
		s->read(d, sizeof(char) * 4);

		char b[4];

		if (is_big_endian() == endian_is_big)
		{
			for (int i = 0; i < 4; ++i)
				b[i] = d[i];
		}
		else
		{
			for (int i = 0; i < 4; ++i)
				b[i] = d[3 - i];
		}

		std::copy(static_cast<const char*>(static_cast<const void*>(b)),
			static_cast<const char*>(static_cast<const void*>(b)) + (sizeof(char) * 4),
			static_cast<char*>(static_cast<void*>(&t)));
	}

	void BinarySerializer::readDouble(double& t, std::istream* s)
	{
		char d[8];
		s->read(d, sizeof(char) * 8);

		char b[8];

		if (is_big_endian() == endian_is_big)
		{
			for (int i = 0; i < 8; ++i)
				b[i] = d[i];
		}
		else
		{
			for (int i = 0; i < 8; ++i)
				b[i] = d[7 - i];
		}

		std::copy(static_cast<const char*>(static_cast<const void*>(b)),
			static_cast<const char*>(static_cast<const void*>(b)) + (sizeof(char) * 8),
			static_cast<char*>(static_cast<void*>(&t)));
	}

	void BinarySerializer::readBool(bool& t, std::istream* s)
	{
		char byte = (char)0;
		s->read(&byte, sizeof(char));
		t = byte == (char)1 ? true : false;
	}

	void BinarySerializer::readChar(char& t, std::istream* s)
	{
		s->read(&t, sizeof(char));
	}

	void BinarySerializer::readUChar(unsigned char& t, std::istream* s)
	{
		char c = 0;
		s->read(&c, sizeof(char));
		t = static_cast<unsigned char>(c);
	}

	void BinarySerializer::readString(std::string& t, std::istream* s)
	{
		int len = 0;
		read<int>(len, s);
		t.resize(len, 0);

		s->read(&t[0], sizeof(char) * len);
	}

	void BinarySerializer::readULong(unsigned long long& t, std::istream* s)
	{
		char d[8];
		s->read(d, sizeof(char) * 8);

		char b[8];

		if (is_big_endian() == endian_is_big)
		{
			for (int i = 0; i < 8; ++i)
				b[i] = d[i];
		}
		else
		{
			for (int i = 0; i < 8; ++i)
				b[i] = d[7 - i];
		}

		std::copy(static_cast<const char*>(static_cast<const void*>(b)),
			static_cast<const char*>(static_cast<const void*>(b)) + (sizeof(char) * 8),
			static_cast<char*>(static_cast<void*>(&t)));
	}

	void BinarySerializer::readVectorInt(std::vector<int>& t, std::istream* s)
	{
		int size = 0;
		read(size, s);

		t.resize(size);

		char* buf = new char[4 * size];
		s->read(buf, sizeof(char) * 4 * size);

		char* bufRev = new char[4 * size];

		if (is_big_endian() == endian_is_big)
		{
			for (int i = 0; i < 4 * size; ++i)
				bufRev[i] = buf[i];
		}
		else
		{
			for (int i = 0; i < 4 * size; ++i)
				bufRev[i] = buf[(4 * size - 1) - i];
		}

		std::copy(static_cast<const char*>(static_cast<const void*>(bufRev)),
			static_cast<const char*>(static_cast<const void*>(bufRev)) + (sizeof(char) * 4 * size),
			static_cast<char*>(static_cast<void*>(&t[0])));

		delete[] buf;
		delete[] bufRev;
	}

	void BinarySerializer::readVectorUInt(std::vector<unsigned int>& t, std::istream* s)
	{
		int size = 0;
		read(size, s);

		t.resize(size);

		char* buf = new char[4 * size];
		s->read(buf, sizeof(char) * 4 * size);

		char* bufRev = new char[4 * size];

		if (is_big_endian() == endian_is_big)
		{
			for (int i = 0; i < 4 * size; ++i)
				bufRev[i] = buf[i];
		}
		else
		{
			for (int i = 0; i < 4 * size; ++i)
				bufRev[i] = buf[(4 * size - 1) - i];
		}

		std::copy(static_cast<const char*>(static_cast<const void*>(bufRev)),
			static_cast<const char*>(static_cast<const void*>(bufRev)) + (sizeof(char) * 4 * size),
			static_cast<char*>(static_cast<void*>(&t[0])));

		delete[] buf;
		delete[] bufRev;
	}

	void BinarySerializer::readVectorFloat(std::vector<float>& t, std::istream* s)
	{
		int size = 0;
		read(size, s);

		t.resize(size);

		char* buf = new char[4 * size];
		s->read(buf, sizeof(char) * 4 * size);

		char* bufRev = new char[4 * size];

		if (is_big_endian() == endian_is_big)
		{
			for (int i = 0; i < 4 * size; ++i)
				bufRev[i] = buf[i];
		}
		else
		{
			for (int i = 0; i < 4 * size; ++i)
				bufRev[i] = buf[(4 * size - 1) - i];
		}

		std::copy(static_cast<const char*>(static_cast<const void*>(bufRev)),
			static_cast<const char*>(static_cast<const void*>(bufRev)) + (sizeof(char) * 4 * size),
			static_cast<char*>(static_cast<void*>(&t[0])));

		delete[] buf;
		delete[] bufRev;
	}

	void BinarySerializer::readVectorUChar(std::vector<unsigned char>& t, std::istream* s)
	{
		int size = 0;
		read(size, s);

		t.resize(size);

		char* buf = new char[size];
		s->read(buf, size * sizeof(char));
		
		memcpy(&t[0], buf, size * sizeof(char));
		delete[] buf;
	}

	void BinarySerializer::readVectorULong(std::vector<unsigned long long>& t, std::istream* s)
	{
		int size = 0;
		read(size, s);

		t.resize(size);

		char* buf = new char[8 * size];
		s->read(buf, sizeof(char) * 8 * size);

		char* bufRev = new char[8 * size];

		if (is_big_endian() == endian_is_big)
		{
			for (int i = 0; i < 8 * size; ++i)
				bufRev[i] = buf[i];
		}
		else
		{
			for (int i = 0; i < 8 * size; ++i)
				bufRev[i] = buf[(8 * size - 1) - i];
		}

		std::copy(static_cast<const char*>(static_cast<const void*>(bufRev)),
			static_cast<const char*>(static_cast<const void*>(bufRev)) + (sizeof(char) * 8 * size),
			static_cast<char*>(static_cast<void*>(&t[0])));

		delete[] buf;
		delete[] bufRev;
	}
}