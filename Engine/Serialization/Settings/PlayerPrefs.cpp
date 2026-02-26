#include "PlayerPrefs.h"

#include <boost/serialization/export.hpp>
#include "../../Classes/IO.h"

namespace GX
{
	PlayerPrefs PlayerPrefs::Singleton;

	void PlayerPrefs::load(std::string path)
	{
		if (IO::FileExists(path))
		{
			std::ifstream ofs(path, std::ios::binary);
			BinarySerializer s;
			s.deserialize(&ofs, this, "Player Prefs");
			ofs.close();
		}
	}

	void PlayerPrefs::save(std::string path)
	{
		std::ofstream ofs(path, std::ios::binary);
		BinarySerializer s;
		s.serialize(&ofs, this, "Player Prefs");
		ofs.close();
	}

	void PlayerPrefs::setInt(std::string key, int value)
	{
		auto it = find_if(dataStorage.begin(), dataStorage.end(), [=](PlayerPrefsData& dt) -> bool
		{
			return dt.key == key;
		});

		if (it != dataStorage.end())
		{
			it->intVal = value;
		}
		else
		{
			PlayerPrefsData dt;
			dt.key = key;
			dt.intVal = value;
			dataStorage.push_back(dt);
		}
	}

	int PlayerPrefs::getInt(std::string key, int defaultValue)
	{
		auto it = find_if(dataStorage.begin(), dataStorage.end(), [=](PlayerPrefsData& dt) -> bool
		{
			return dt.key == key;
		});

		if (it != dataStorage.end())
		{
			return it->intVal;
		}

		return defaultValue;
	}

	void PlayerPrefs::setFloat(std::string key, float value)
	{
		auto it = find_if(dataStorage.begin(), dataStorage.end(), [=](PlayerPrefsData& dt) -> bool
		{
			return dt.key == key;
		});

		if (it != dataStorage.end())
		{
			it->floatVal = value;
		}
		else
		{
			PlayerPrefsData dt;
			dt.key = key;
			dt.floatVal = value;
			dataStorage.push_back(dt);
		}
	}

	float PlayerPrefs::getFloat(std::string key, float defaultValue)
	{
		auto it = find_if(dataStorage.begin(), dataStorage.end(), [=](PlayerPrefsData& dt) -> bool
		{
			return dt.key == key;
		});

		if (it != dataStorage.end())
		{
			return it->floatVal;
		}

		return defaultValue;
	}

	void PlayerPrefs::setString(std::string key, std::string value)
	{
		auto it = find_if(dataStorage.begin(), dataStorage.end(), [=](PlayerPrefsData& dt) -> bool
		{
			return dt.key == key;
		});

		if (it != dataStorage.end())
		{
			it->stringVal = value;
		}
		else
		{
			PlayerPrefsData dt;
			dt.key = key;
			dt.stringVal = value;
			dataStorage.push_back(dt);
		}
	}

	std::string PlayerPrefs::getString(std::string key, std::string defaultValue)
	{
		auto it = find_if(dataStorage.begin(), dataStorage.end(), [=](PlayerPrefsData& dt) -> bool
		{
			return dt.key == key;
		});

		if (it != dataStorage.end())
		{
			return it->stringVal;
		}

		return defaultValue;
	}

	bool PlayerPrefs::hasKey(std::string key)
	{
		auto it = find_if(dataStorage.begin(), dataStorage.end(), [=](PlayerPrefsData& dt) -> bool
		{
			return dt.key == key;
		});

		if (it != dataStorage.end())
		{
			return true;
		}

		return false;
	}

	void PlayerPrefs::deleteKey(std::string key)
	{
		auto it = find_if(dataStorage.begin(), dataStorage.end(), [=](PlayerPrefsData& dt) -> bool
		{
			return dt.key == key;
		});

		if (it != dataStorage.end())
		{
			dataStorage.erase(it);
		}
	}

	void PlayerPrefs::deleteAll()
	{
		dataStorage.clear();
	}
}