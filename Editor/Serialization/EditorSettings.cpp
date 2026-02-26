#include "EditorSettings.h"

#include "../Engine/Classes/IO.h"
#include "../Engine/Core/Debug.h"

namespace GX
{
	EditorSettings::EditorSettings()
	{
	}

	EditorSettings::~EditorSettings()
	{
	}

	void EditorSettings::save(std::string path)
	{
		std::ofstream ofs(path, std::ios::binary);
		BinarySerializer s;
		s.serialize(&ofs, this, "Editor Settings");
		ofs.close();

		loadedPath = path;
	}

	void EditorSettings::save()
	{
		save(loadedPath);
	}

	void EditorSettings::load(std::string path)
	{
		if (IO::FileExists(path))
		{
			//Deserialize
			try
			{
				std::ifstream ofs(path, std::ios::binary);
				BinarySerializer s;
				s.deserialize(&ofs, this, "Editor Settings");
				ofs.close();
			}
			catch(const std::exception& e)
			{
				Debug::logError(std::string("Editor settings load error: ") + e.what());
				std::cerr << "Editor settings load error: " << e.what() << std::endl;
			}
		}

		loadedPath = path;
	}
}