#pragma once

#include <ostream>
#include <istream>
#include <vector>

#include "Serializer.h"

namespace GX
{
	class BinarySerializer : public Serializer
	{
	public:
		BinarySerializer() : Serializer() {}
		virtual ~BinarySerializer() {}

		//Write
		virtual void writeInt(int& t, std::ostream* s);
		virtual void writeUInt(unsigned int& t, std::ostream* s);
		virtual void writeFloat(float& t, std::ostream* s);
		virtual void writeDouble(double& t, std::ostream* s);
		virtual void writeBool(bool& t, std::ostream* s);
		virtual void writeChar(char& t, std::ostream* s);
		virtual void writeUChar(unsigned char& t, std::ostream* s);
		virtual void writeString(std::string& t, std::ostream* s);
		virtual void writeULong(unsigned long long& t, std::ostream* s);

		virtual void writeVectorInt(std::vector<int>& t, std::ostream* s);
		virtual void writeVectorUInt(std::vector<unsigned int>& t, std::ostream* s);
		virtual void writeVectorFloat(std::vector<float>& t, std::ostream* s);
		virtual void writeVectorUChar(std::vector<unsigned char>& t, std::ostream* s);
		virtual void writeVectorULong(std::vector<unsigned long long>& t, std::ostream* s);

		//Read
		virtual void readInt(int& t, std::istream* s);
		virtual void readUInt(unsigned int& t, std::istream* s);
		virtual void readFloat(float& t, std::istream* s);
		virtual void readDouble(double& t, std::istream* s);
		virtual void readBool(bool& t, std::istream* s);
		virtual void readChar(char& t, std::istream* s);
		virtual void readUChar(unsigned char& t, std::istream* s);
		virtual void readString(std::string& t, std::istream* s);
		virtual void readULong(unsigned long long& t, std::istream* s);

		virtual void readVectorInt(std::vector<int>& t, std::istream* s);
		virtual void readVectorUInt(std::vector<unsigned int>& t, std::istream* s);
		virtual void readVectorFloat(std::vector<float>& t, std::istream* s);
		virtual void readVectorUChar(std::vector<unsigned char>& t, std::istream* s);
		virtual void readVectorULong(std::vector<unsigned long long>& t, std::istream* s);
	};
}