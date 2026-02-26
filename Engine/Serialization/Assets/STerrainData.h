#pragma once

#include <string>
#include <vector>

#include "../Data/SVector.h"
#include "../Data/SMatrix4.h"

//----Splat textures----//

namespace GX
{
	struct STerrainSplatTexture : public Archive
	{
	public:
		virtual void serialize(Serializer* s)
		{
			Archive::serialize(s);
			dataVector(pixels);
		}

		STerrainSplatTexture() {}
		~STerrainSplatTexture() {}

		std::vector<unsigned char> pixels;
	};

	//----Model meshes info----//

	struct STerrainModelMeshData : public Archive
	{
	public:
		virtual void serialize(Serializer* s)
		{
			Archive::serialize(s);
			data(mesh);
			data(transform);
			data(materials);
		}

		STerrainModelMeshData() {}
		~STerrainModelMeshData() {}

		std::string mesh;
		SMatrix4 transform = SMatrix4(glm::identity<glm::mat4x4>());
		std::vector<std::string> materials;
	};

	//----Textures----//

	struct STerrainTextureData : public Archive
	{
	public:
		virtual void serialize(Serializer* s)
		{
			Archive::serialize(s);
			data(diffuseTexture);
			data(normalTexture);
			data(worldSize);
		}

		STerrainTextureData() {}
		~STerrainTextureData() {}

		std::string diffuseTexture = "";
		std::string normalTexture = "";
		float worldSize = 100.0f;
	};

	//----Trees----//

	struct STerrainTreeMeshData : public Archive
	{
	public:
		virtual void serialize(Serializer* s)
		{
			Archive::serialize(s);
			data(position);
			data(scale);
			data(rotation);
		}

		STerrainTreeMeshData() {}
		~STerrainTreeMeshData() {}

		SVector3 position = SVector3(0.0f, 0.0f, 0.0f);
		float scale = 1.0f;
		float rotation = 0.0f;
	};

	struct STerrainTreeData : public Archive
	{
	public:
		virtual void serialize(Serializer* s)
		{
			Archive::serialize(s);
			data(name);
			data(minScale);
			data(maxScale);
			data(meshes);
			data(meshModels);
		}

		STerrainTreeData() {}
		~STerrainTreeData() {}

		std::string name = "";
		float minScale = 1.0f;
		float maxScale = 1.5f;
		std::vector<STerrainTreeMeshData> meshes;
		std::vector<STerrainModelMeshData> meshModels;
	};

	//----Detail meshes----//

	struct STerrainDetailMeshInfoData : public Archive
	{
	public:
		virtual void serialize(Serializer* s)
		{
			Archive::serialize(s);
			data(position);
			data(scale);
			data(rotation);
		}

		STerrainDetailMeshInfoData() {}
		~STerrainDetailMeshInfoData() {}

		SVector3 position = SVector3(0.0f, 0.0f, 0.0f);
		float scale = 1.0f;
		float rotation = 0.0f;
	};

	struct STerrainDetailMeshData : public Archive
	{
	public:
		virtual void serialize(Serializer* s)
		{
			Archive::serialize(s);
			data(name);
			data(minScale);
			data(maxScale);
			data(meshes);
			data(meshModels);
		}

		STerrainDetailMeshData() {}
		~STerrainDetailMeshData() {}

		std::string name = "";
		float minScale = 1.0f;
		float maxScale = 1.5f;
		std::vector<STerrainDetailMeshInfoData> meshes;
		std::vector<STerrainModelMeshData> meshModels;
	};

	//----Grass----//

	struct STerrainGrassMeshData : public Archive
	{
	public:
		virtual void serialize(Serializer* s)
		{
			Archive::serialize(s);
			data(position);
			data(rotation);
			data(width);
			data(height);
		}

		STerrainGrassMeshData() {}
		~STerrainGrassMeshData() {}

		SVector3 position = SVector3(0.0f, 0.0f, 0.0f);
		float rotation = 0.0f;
		float width = 0.0f;
		float height = 0.0f;
	};

	struct STerrainGrassData : public Archive
	{
	public:
		virtual void serialize(Serializer* s)
		{
			Archive::serialize(s);
			data(name);
			data(minSize);
			data(maxSize);
			data(meshes);
		}

		STerrainGrassData() {}
		~STerrainGrassData() {}

		std::string name = "";
		SVector2 minSize = SVector2(1, 1);
		SVector2 maxSize = SVector2(2, 2);
		std::vector<STerrainGrassMeshData> meshes;
	};

	//----Terrain----//

	class STerrainData : public Archive
	{
	public:
		virtual void serialize(Serializer* s)
		{
			Archive::serialize(s);
			data(material);
			data(size);
			data(worldSize);
			data(treeImpostorStartDistance);
			data(treeDrawDistance);
			data(detailMeshesDrawDistance);
			data(grassDrawDistance);
			data(drawGrass);
			data(drawTrees);
			data(drawDetailMeshes);
			data(grassCastShadows);
			data(treesCastShadows);
			data(detailMeshesCastShadows);
			data(terrainCastShadows);
			data(splatTextures);
			data(textureList);
			data(treeList);
			data(detailMeshList);
			data(grassList);
			dataVector(heightMap);
		}

		STerrainData() {}
		~STerrainData() {}

		std::string material = "";
		int size = 128;
		int worldSize = 256;
		float treeImpostorStartDistance = 90.0f;
		float treeDrawDistance = 250.0f;
		float detailMeshesDrawDistance = 40.0f;
		float grassDrawDistance = 50.0f;
		bool drawGrass = true;
		bool drawTrees = true;
		bool drawDetailMeshes = true;
		bool terrainCastShadows = true;
		bool grassCastShadows = true;
		bool treesCastShadows = true;
		bool detailMeshesCastShadows = true;

		std::vector<float> heightMap;
		std::vector<STerrainSplatTexture> splatTextures;
		std::vector<STerrainTreeData> treeList;
		std::vector<STerrainTextureData> textureList;
		std::vector<STerrainDetailMeshData> detailMeshList;
		std::vector<STerrainGrassData> grassList;
	};
}