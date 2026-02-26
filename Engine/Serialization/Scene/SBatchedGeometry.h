#pragma once

#include <vector>
#include <string>

#include "../Data/SVector.h"
#include "../Data/SColor.h"

namespace GX
{
	class SBatchVertexBuffer : public Archive
	{
	public:

		virtual void serialize(Serializer* s)
		{
			Archive::serialize(s);
			data(position);
			data(normal);
			data(tangent);
			data(bitangent);
			data(texcoord0);
			data(texcoord1);
			data(color);
		}

		SBatchVertexBuffer() {}
		~SBatchVertexBuffer() {}

		SVector3 position;
		SVector3 normal;
		SVector3 tangent;
		SVector3 bitangent;
		SVector2 texcoord0;
		SVector2 texcoord1;
		SColor color;
	};

	class SBatch : public Archive
	{
	public:
		SBatch() {}
		~SBatch()
		{
			vertexBuffer.clear();
			indexBuffer.clear();
		}

		virtual void serialize(Serializer* s)
		{
			Archive::serialize(s);
			data(material);
			data(guid);
			data(transparent);
			data(lightingStatic);
			data(useLightmapUVs);
			data(castShadows);
			data(lightmapSize);
			data(layer);
			data(index);
			data(vertexBuffer);
			dataVector(indexBuffer);
		}

		//Serialize members
		std::vector<SBatchVertexBuffer> vertexBuffer;
		std::vector<uint32_t> indexBuffer;

		std::string material = "";
		std::string guid = "";

		bool transparent = false;
		bool lightingStatic = false;
		bool useLightmapUVs = false;
		bool castShadows = true;
		uint8_t lightmapSize = 0;

		int layer = 0;
		int index = 0;

		void clear();
	};

	class SBatchedGeometry : public Archive
	{
	public:
		SBatchedGeometry() {}
		~SBatchedGeometry() {}

		virtual void serialize(Serializer* s)
		{
			Archive::serialize(s);
			data(batches);
		}

		//Serialize members
		std::vector<SBatch> batches;

		void clear();

		void save(std::string location, std::string name);
		bool load(std::string location, std::string name);
	};
}