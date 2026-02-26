#pragma once

#include <string>
#include <vector>

#include "../Data/SMatrix4.h"
#include "../Data/SColor.h"

#include "../Serializers/BinarySerializer.h"

namespace GX
{
	class SVertexBuffer : public Archive
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
			data(blendWeights);
			data(blendIndices);
			data(color);
		}

		SVertexBuffer() {}
		~SVertexBuffer() {}

		SVector3 position;
		SVector3 normal;
		SVector3 tangent;
		SVector3 bitangent;
		SVector2 texcoord0;
		SVector2 texcoord1;
		SVector4 blendWeights;
		SVector4 blendIndices;
		SColor color;
	};

	class SBoneInfo : public Archive
	{
	public:
		virtual void serialize(Serializer* s)
		{
			Archive::serialize(s);
			data(name);
			data(offsetMatrix);
		}

		SBoneInfo() {}
		~SBoneInfo() {}

		std::string name = "";
		SMatrix4 offsetMatrix = SMatrix4(glm::identity<glm::mat4x4>());
	};

	class SLodInfo : public Archive
	{
	public:
		virtual void serialize(Serializer* s)
		{
			Archive::serialize(s);
			dataVector(indexBuffer);
		}

		SLodInfo() {}
		~SLodInfo() {}

		std::vector<uint32_t> indexBuffer;
	};

	class SSubMesh : public Archive
	{
	public:
		virtual void serialize(Serializer* s)
		{
			Archive::serialize(s);
			data(materialName);
			data(useLightmapUVs);
			data(vertexBuffer);
			dataVector(indexBuffer);
			data(bones);
			data(lods);
		}

		SSubMesh() {}
		~SSubMesh()
		{
			vertexBuffer.clear();
			indexBuffer.clear();
			lods.clear();
			bones.clear();
		}

		std::string materialName = "";
		bool useLightmapUVs = false;

		std::vector<SVertexBuffer> vertexBuffer;
		std::vector<uint32_t> indexBuffer;
		std::vector<SLodInfo> lods;
		std::vector<SBoneInfo> bones;
	};

	class SMesh : public Archive
	{
	public:
		virtual void serialize(Serializer* s)
		{
			Archive::serialize(s);
			data(alias);
			data(sourceFile);
			data(subMeshes);
		}

		SMesh() {}
		~SMesh() {}

		std::string alias = "";
		std::string sourceFile = "";
		std::vector<SSubMesh> subMeshes;
	};
}