#pragma once

#include <ostream>
#include <istream>
#include <vector>
#include <string>

namespace GX
{
	class Serializer;

	class Archive
	{
	private:
		Serializer* serializer = nullptr;

	protected:
		int version = 0;

	public:
		Archive() {}
		virtual ~Archive() {}

		template<typename T>
		void data(T& t);

		template<typename T>
		void dataVector(T& t);

		virtual void serialize(Serializer* s);
		virtual int getVersion() { return 0; }
	};

	class Serializer
	{
	public:
		enum class Operation
		{
			Serialize,
			Deserialize
		};

	protected:
		Operation operation = Operation::Serialize;
		void* stream = nullptr;

		std::string fileHeader = "Falco Engine";
		bool endian_is_big = true;
		int version = 0;

		bool is_big_endian(void);

		void checkInputStream(std::istream* s);
		void checkOutputStream(std::ostream* s);

	public:
		Serializer() {}
		virtual ~Serializer() {}

		int getVersion() { return version; }
		Operation getOperation() { return operation; }

		template<typename T>
		inline void data(T& t)
		{
			if (operation == Operation::Serialize)
				write(t, (std::ostream*)stream);
			else
				read(t, (std::istream*)stream);
		}

		template<typename T>
		inline void dataVector(T& t)
		{
			if (operation == Operation::Serialize)
				writeVector(t, (std::ostream*)stream);
			else
				readVector(t, (std::istream*)stream);
		}

		//

		template<typename T>
		inline void write(T& t, std::ostream* s) { t.serialize(this); }

		template<typename T>
		inline void read(T& t, std::istream* s) { t.serialize(this); }

		template<typename T>
		inline void write(std::vector<T>& t, std::ostream* s)
		{
			int size = (int)t.size();
			write(size, s);

			for (int i = 0; i < size; ++i)
				write<T>(t[i], s);
		}

		template<typename T>
		inline void read(std::vector<T>& t, std::istream* s)
		{
			int size = 0;
			read(size, s);

			t.resize(size);

			for (int i = 0; i < size; ++i)
				read<T>(t[i], s);
		}

		template<typename T>
		inline void writeVector(std::vector<T>& t, std::ostream* s) {}

		template<typename T>
		inline void readVector(std::vector<T>& t, std::istream* s) {}

		virtual void writeInt(int& t, std::ostream* s) = 0;
		virtual void writeUInt(unsigned int& t, std::ostream* s) = 0;
		virtual void writeFloat(float& t, std::ostream* s) = 0;
		virtual void writeDouble(double& t, std::ostream* s) = 0;
		virtual void writeBool(bool& t, std::ostream* s) = 0;
		virtual void writeChar(char& t, std::ostream* s) = 0;
		virtual void writeUChar(unsigned char& t, std::ostream* s) = 0;
		virtual void writeString(std::string& t, std::ostream* s) = 0;
		virtual void writeULong(unsigned long long& t, std::ostream* s) = 0;

		virtual void writeVectorInt(std::vector<int>& t, std::ostream* s) = 0;
		virtual void writeVectorUInt(std::vector<unsigned int>& t, std::ostream* s) = 0;
		virtual void writeVectorFloat(std::vector<float>& t, std::ostream* s) = 0;
		virtual void writeVectorUChar(std::vector<unsigned char>& t, std::ostream* s) = 0;
		virtual void writeVectorULong(std::vector<unsigned long long>& t, std::ostream* s) = 0;

		virtual void readInt(int& t, std::istream* s) = 0;
		virtual void readUInt(unsigned int& t, std::istream* s) = 0;
		virtual void readFloat(float& t, std::istream* s) = 0;
		virtual void readDouble(double& t, std::istream* s) = 0;
		virtual void readBool(bool& t, std::istream* s) = 0;
		virtual void readChar(char& t, std::istream* s) = 0;
		virtual void readUChar(unsigned char& t, std::istream* s) = 0;
		virtual void readString(std::string& t, std::istream* s) = 0;
		virtual void readULong(unsigned long long& t, std::istream* s) = 0;

		virtual void readVectorInt(std::vector<int>& t, std::istream* s) = 0;
		virtual void readVectorUInt(std::vector<unsigned int>& t, std::istream* s) = 0;
		virtual void readVectorFloat(std::vector<float>& t, std::istream* s) = 0;
		virtual void readVectorUChar(std::vector<unsigned char>& t, std::istream* s) = 0;
		virtual void readVectorULong(std::vector<unsigned long long>& t, std::istream* s) = 0;

		void serialize(std::ostream* s, Archive* a, std::string fileFormat);
		void deserialize(std::istream* s, Archive* a, std::string fileFormat);
	};

	template<typename T>
	inline void Archive::data(T& t)
	{
		serializer->data(t);
	}

	template<typename T>
	inline void Archive::dataVector(T& t)
	{
		serializer->dataVector(t);
	}

	//Write

	template<>
	inline void Serializer::write<int>(int& t, std::ostream* s)
	{
		writeInt(t, s);
		checkOutputStream(s);
	}

	template<>
	inline void Serializer::write<unsigned int>(unsigned int& t, std::ostream* s)
	{
		writeUInt(t, s);
		checkOutputStream(s);
	}

	template<>
	inline void Serializer::write<float>(float& t, std::ostream* s)
	{
		writeFloat(t, s);
		checkOutputStream(s);
	}

	template<>
	inline void Serializer::write<double>(double& t, std::ostream* s)
	{
		writeDouble(t, s);
		checkOutputStream(s);
	}

	template<>
	inline void Serializer::write<bool>(bool& t, std::ostream* s)
	{
		writeBool(t, s);
		checkOutputStream(s);
	}

	template<>
	inline void Serializer::write<char>(char& t, std::ostream* s)
	{
		writeChar(t, s);
		checkOutputStream(s);
	}

	template<>
	inline void Serializer::write<unsigned char>(unsigned char& t, std::ostream* s)
	{
		writeUChar(t, s);
		checkOutputStream(s);
	}

	template<>
	inline void Serializer::write<std::string>(std::string& t, std::ostream* s)
	{
		writeString(t, s);
		checkOutputStream(s);
	}

	template<>
	inline void Serializer::write<unsigned long long>(unsigned long long& t, std::ostream* s)
	{
		writeULong(t, s);
		checkOutputStream(s);
	}

	template<>
	inline void Serializer::writeVector<int>(std::vector<int>& t, std::ostream* s)
	{
		writeVectorInt(t, s);
		checkOutputStream(s);
	}

	template<>
	inline void Serializer::writeVector<unsigned int>(std::vector<unsigned int>& t, std::ostream* s)
	{
		writeVectorUInt(t, s);
		checkOutputStream(s);
	}

	template<>
	inline void Serializer::writeVector<float>(std::vector<float>& t, std::ostream* s)
	{
		writeVectorFloat(t, s);
		checkOutputStream(s);
	}

	template<>
	inline void Serializer::writeVector<unsigned char>(std::vector<unsigned char>& t, std::ostream* s)
	{
		writeVectorUChar(t, s);
		checkOutputStream(s);
	}
	
	template<>
	inline void Serializer::writeVector<unsigned long long>(std::vector<unsigned long long>& t, std::ostream* s)
	{
		writeVectorULong(t, s);
		checkOutputStream(s);
	}

	//Read

	template<>
	inline void Serializer::read<int>(int& t, std::istream* s)
	{
		checkInputStream(s);
		readInt(t, s);
	}

	template<>
	inline void Serializer::read<unsigned int>(unsigned int& t, std::istream* s)
	{
		checkInputStream(s);
		readUInt(t, s);
	}

	template<>
	inline void Serializer::read<float>(float& t, std::istream* s)
	{
		checkInputStream(s);
		readFloat(t, s);
	}

	template<>
	inline void Serializer::read<double>(double& t, std::istream* s)
	{
		checkInputStream(s);
		readDouble(t, s);
	}

	template<>
	inline void Serializer::read<bool>(bool& t, std::istream* s)
	{
		checkInputStream(s);
		readBool(t, s);
	}

	template<>
	inline void Serializer::read<char>(char& t, std::istream* s)
	{
		checkInputStream(s);
		readChar(t, s);
	}

	template<>
	inline void Serializer::read<unsigned char>(unsigned char& t, std::istream* s)
	{
		checkInputStream(s);
		readUChar(t, s);
	}

	template<>
	inline void Serializer::read<std::string>(std::string& t, std::istream* s)
	{
		checkInputStream(s);
		readString(t, s);
	}

	template<>
	inline void Serializer::read<unsigned long long>(unsigned long long& t, std::istream* s)
	{
		checkInputStream(s);
		readULong(t, s);
	}

	template<>
	inline void Serializer::readVector<int>(std::vector<int>& t, std::istream* s)
	{
		checkInputStream(s);
		readVectorInt(t, s);
	}

	template<>
	inline void Serializer::readVector<unsigned int>(std::vector<unsigned int>& t, std::istream* s)
	{
		checkInputStream(s);
		readVectorUInt(t, s);
	}

	template<>
	inline void Serializer::readVector<float>(std::vector<float>& t, std::istream* s)
	{
		checkInputStream(s);
		readVectorFloat(t, s);
	}

	template<>
	inline void Serializer::readVector<unsigned char>(std::vector<unsigned char>& t, std::istream* s)
	{
		checkInputStream(s);
		readVectorUChar(t, s);
	}

	template<>
	inline void Serializer::readVector<unsigned long long>(std::vector<unsigned long long>& t, std::istream* s)
	{
		checkInputStream(s);
		readVectorULong(t, s);
	}
}