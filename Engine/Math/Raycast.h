#pragma once

#include <utility>
#include <bitset>
#include <string>
#include <vector>
#include <map>

#include "../glm/vec3.hpp"
#include "../glm/vec4.hpp"
#include "../glm/mat4x4.hpp"

#include "../Renderer/Color.h"
#include "../Renderer/BatchedGeometry.h"
#include "../Renderer/CSGGeometry.h"

#include "Ray.h"
#include "../Core/LayerMask.h"

namespace GX
{
	class MeshRenderer;
	class Transform;
	class Terrain;
	class MeshRenderer;

	class Raycast
	{
	public:
		struct HitInfo
		{
		public:
			Transform* object = nullptr;
			MeshRenderer* renderer = nullptr;
			BatchedGeometry::Batch* batch = nullptr;
			CSGGeometry::SubMesh* csgSubMesh = nullptr;
			int subMeshIndex = 0;
			glm::vec3 hitPoint = glm::vec3(0, 0, 0);
			glm::vec3 hitNormal = glm::vec3(0, 0, 0);
			glm::vec3 hitTrianglePosition[3];
			glm::vec3 hitTriangleNormal[3];
			glm::vec2 hitTriangleTexCoord0[3];
			glm::vec2 hitTriangleTexCoord1[3];
			Material* material = nullptr;
			float distance = 0.0f;
			size_t csgBrushId = -1;
		};

	private:
		bool hitTestMesh(MeshRenderer* meshRenderer, Ray ray, Raycast::HitInfo& inf);
		bool hitTestBatch(BatchedGeometry::Batch* batch, Ray ray, Raycast::HitInfo& inf);
		bool hitTestCSG(CSGGeometry::SubMesh* subMesh, Ray ray, Raycast::HitInfo& inf);
		bool hitTestTerrain(Terrain * terrain, Ray ray, Raycast::HitInfo& inf);
		std::pair<bool, Raycast::HitInfo> hitTestTriangle(glm::vec3& pp1, glm::vec3& pp2, glm::vec3& pp3, glm::mat4x4& mtx, Ray& ray);
		
	public:
		bool checkLightingStatic = false;
		bool lightingStatic = false;
		bool raycastTransientRenderables = true;
		bool checkCastShadows = false;
		bool castShadows = true;
		bool fetchAdditionalInfo = false;
		bool checkBatchedGeometry = false;
		bool checkCSGGeometry = false;
		bool findCSGBrushes = false;

		std::vector<Raycast::HitInfo> execute(Ray ray, LayerMask layerMask = LayerMask());
		std::vector<Raycast::HitInfo> executeBoundsOnly(Ray ray);
	};
}