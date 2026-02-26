#pragma once

#include <string>
#include <vector>

#include "../Serializers/BinarySerializer.h"

namespace GX
{
	class PlayerPrefsData : public Archive
	{
	public:
		PlayerPrefsData() = default;
		~PlayerPrefsData() = default;

		virtual void serialize(Serializer* s)
		{
			Archive::serialize(s);
			data(key);
			data(stringVal);
			data(intVal);
			data(floatVal);
		}

		std::string key = "";
		std::string stringVal = "";
		int intVal = 0;
		float floatVal = 0;
	};

	class PlayerPrefs : public Archive
	{
	private:
		std::vector<PlayerPrefsData> dataStorage;

	public:
		PlayerPrefs() = default;
		~PlayerPrefs() = default;

		virtual void serialize(Serializer* s)
		{
			Archive::serialize(s);
			data(dataStorage);
		}

		void load(std::string path);
		void save(std::string path);

		void setInt(std::string key, int value);
		int getInt(std::string key, int defaultValue = 0);

		void setFloat(std::string key, float value);
		float getFloat(std::string key, float defaultValue = 0);

		void setString(std::string key, std::string value);
		std::string getString(std::string key, std::string defaultValue = "");

		bool hasKey(std::string key);
		void deleteKey(std::string key);
		void deleteAll();

		static PlayerPrefs Singleton;
	};
}