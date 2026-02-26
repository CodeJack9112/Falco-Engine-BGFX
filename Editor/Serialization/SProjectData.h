#pragma once

#include <string>
#include <vector>

#include "../Engine/Serialization/Serializers/BinarySerializer.h"

namespace GX
{
	struct SProject : public Archive
	{
	public:
		SProject() {}
		~SProject() {}

		friend bool operator==(const SProject& l, const SProject& r)
		{
			return l.projectPath == r.projectPath;
		}

		virtual void serialize(Serializer* s)
		{
			Archive::serialize(s);
			data(projectName);
			data(projectPath);
			data(engineVersion);
		}

	public:
		std::string projectName;
		std::string projectPath;
		std::string engineVersion;
	};

	struct SProjectInfo : public Archive
	{
	public:
		SProjectInfo() {}
		~SProjectInfo() {}

		virtual void serialize(Serializer* s)
		{
			Archive::serialize(s);
			data(projects);
		}

	public:
		std::vector<SProject> projects;
	};
}