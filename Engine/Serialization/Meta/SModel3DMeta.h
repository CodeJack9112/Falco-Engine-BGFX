#pragma once

#include <string>

#include "../Serializers/BinarySerializer.h"

namespace GX
{
	class SModel3DMeta : public Archive
	{
	public:
		virtual int getVersion() { return 0; }

		virtual void serialize(Serializer* s)
		{
			Archive::serialize(s);
			data(calculateNormals);
			data(maxSmoothingAngle);
			data(scale);
			data(optimizeMeshes);
			data(generateLightmapUVs);
			data(generateLod);
			data(lodLevels);
			data(lodError);
			data(lodPreserveMeshTopology);
		}

		SModel3DMeta() {}
		~SModel3DMeta() {}

		float scale = 1.0f; // Global scale
		bool optimizeMeshes = false; // Merge meshes with the same materials
		bool calculateNormals = false; // If false then import from file
		float maxSmoothingAngle = 80.0f; // Max smooth angle when generating normals
		bool generateLightmapUVs = false; // Generate lightmap UVs or not

		bool generateLod = false; // Generate LOD
		int lodLevels = 2; // Lod levels
		float lodError = 0.25f; // Max lod error
		bool lodPreserveMeshTopology = false; // Preserve mesh topology

		std::string filePath = "";

		void save(std::string path);
		void load(std::string path);
	};
}