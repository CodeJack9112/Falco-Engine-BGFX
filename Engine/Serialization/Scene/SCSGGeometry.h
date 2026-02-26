#pragma once

#include <vector>
#include <string>

#include "../Data/SVector.h"
#include "../Data/SColor.h"

namespace GX
{
	class SCSGSubMeshVertexBuffer : public Archive
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

		SCSGSubMeshVertexBuffer() {}
		~SCSGSubMeshVertexBuffer() {}

		SVector3 position;
		SVector3 normal;
		SVector3 tangent;
		SVector3 bitangent;
		SVector2 texcoord0;
		SVector2 texcoord1;
		SColor color;
	};

	class SCSGSubMesh : public Archive
	{
	public:
		SCSGSubMesh() {}
		~SCSGSubMesh()
		{
			clear();
		}

		virtual void serialize(Serializer* s)
		{
			Archive::serialize(s);
			data(material);
			data(guid);
			data(castShadows);
			data(layer);
			data(vertexBuffer);
			dataVector(indexBuffer);
			dataVector(idBuffer);
		}

		//Serialize members
		std::vector<SCSGSubMeshVertexBuffer> vertexBuffer;
		std::vector<uint32_t> indexBuffer;
		std::vector<unsigned long long> idBuffer;

		std::string material = "";
		std::string guid = "";

		bool castShadows = true;
		int layer = 0;

		void clear();
	};

	class SCSGGeometry : public Archive
	{
	public:
		SCSGGeometry() {}
		~SCSGGeometry() {}

		virtual void serialize(Serializer* s)
		{
			Archive::serialize(s);
			data(subMeshes);
		}

		//Serialize members
		std::vector<SCSGSubMesh> subMeshes;

		void clear();

		void save(std::string location, std::string name);
		bool load(std::string location, std::string name);
	};
}