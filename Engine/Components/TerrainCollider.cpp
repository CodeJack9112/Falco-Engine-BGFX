#include "TerrainCollider.h"

#include "../Bullet/include/btBulletCollisionCommon.h"
#include "../Bullet/include/btBulletDynamicsCommon.h"
#include "../Bullet/include/BulletCollision/CollisionShapes/btHeightfieldTerrainShape.h"

#include "../Core/GameObject.h"
#include "../Components/Terrain.h"
#include "../Components/Transform.h"

namespace GX
{
	std::string TerrainCollider::COMPONENT_TYPE = "TerrainCollider";

	TerrainCollider::TerrainCollider() : Collider(nullptr)
	{
		
	}

	TerrainCollider::~TerrainCollider()
	{
		if (pTerrainHeightDataConvert != nullptr)
			delete[] pTerrainHeightDataConvert;
		pTerrainHeightDataConvert = nullptr;
	}

	Component* TerrainCollider::onClone()
	{
		TerrainCollider* newComponent = new TerrainCollider();
		return newComponent;
	}

	void TerrainCollider::onAttach()
	{
		Component::onAttach();

		rebuild();
	}

	void TerrainCollider::onSceneLoaded()
	{
		rebuild();
	}

	void TerrainCollider::rebuild()
	{
		if (collisionShape != nullptr)
			delete collisionShape;
		collisionShape = nullptr;

		if (pTerrainHeightDataConvert != nullptr)
			delete[] pTerrainHeightDataConvert;
		pTerrainHeightDataConvert = nullptr;

		if (getGameObject() == nullptr)
			return;

		Terrain* terrain = (Terrain*)getGameObject()->getComponent(Terrain::COMPONENT_TYPE);
		if (terrain == nullptr)
			return;

		Transform* transform = getGameObject()->getTransform();
		glm::vec3 terrainPosition = transform->getPosition();

		uint32_t terrainSize = terrain->getSize();
		float terrainWorldSize = terrain->getWorldSize();

		float* pTerrainHeightData = terrain->getHeightMap();
		pTerrainHeightDataConvert = new float[terrainSize * terrainSize];

		float _minY = FLT_MAX;
		float _maxY = FLT_MIN;

		for (uint32_t i = 0; i < terrainSize * terrainSize; ++i)
		{
			_minY = std::min(_minY, pTerrainHeightData[i]);
			_maxY = std::max(_maxY, pTerrainHeightData[i]);
		}

		for (uint32_t i = 0; i < terrainSize; ++i)
		{
			for (uint32_t j = 0; j < terrainSize; ++j)
			{
				pTerrainHeightDataConvert[i + (terrainSize * j)] = pTerrainHeightData[(terrainSize - 1 - i) + (terrainSize * j)];
			}
		}

		collisionShape = new btHeightfieldTerrainShape(terrainSize, terrainSize, pTerrainHeightDataConvert, 1.0f, _minY, _maxY, 1, PHY_FLOAT, true);

		float unitsBetweenVertices = (terrainWorldSize / (float)terrainSize);
		scale = glm::vec3(unitsBetweenVertices, 1.0f, unitsBetweenVertices);
		offset = glm::vec3(0.5f, (_minY + _maxY) * 0.5f, -0.5f);

		//((btHeightfieldTerrainShape*)collisionShape)->setUseDiamondSubdivision(true);

		setCollisionShape(collisionShape);
	}
}