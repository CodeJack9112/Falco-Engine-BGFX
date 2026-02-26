#include "Raycast.h"

#include "../Core/Engine.h"
#include "../Renderer/Renderer.h"
#include "../Core/GameObject.h"
#include "../Components/Transform.h"
#include "../Components/MeshRenderer.h"
#include "../Components/Terrain.h"
#include "../Components/CSGBrush.h"

#include "Assets/Mesh.h"
#include "Assets/Material.h"
#include "Assets/Texture.h"

#include "Math/Mathf.h"
#include "Math/AxisAlignedBox.h"

namespace GX
{
	std::vector<Raycast::HitInfo> Raycast::execute(Ray ray, LayerMask layerMask)
	{
		typedef std::pair<GameObject*, Raycast::HitInfo> hitPair;

		std::vector<GameObject*> objects = Engine::getSingleton()->getGameObjects();
		std::vector<TransientRenderable>& renderables = Renderer::getSingleton()->getTransientRenderables();

		std::vector<hitPair> candidates;

		for (auto it = objects.begin(); it != objects.end(); ++it)
		{
			GameObject* obj = *it;

			if (!layerMask.getLayer(obj->getLayer()))
				continue;

			if (!obj->getActive())
				continue;

			if (checkLightingStatic)
			{
				if (obj->getLightingStatic() != lightingStatic || obj->getBatchingStatic())
					continue;
			}

			MeshRenderer* rend = (MeshRenderer*)obj->getComponent(MeshRenderer::COMPONENT_TYPE);
			Terrain* terrain = (Terrain*)obj->getComponent(Terrain::COMPONENT_TYPE);
			
			if (rend != nullptr)
			{
				if (!rend->getEnabled())
					continue;

				if (checkCastShadows)
				{
					if (rend->getCastShadows() != castShadows)
						continue;
				}

				AxisAlignedBox bounds = rend->getBounds(true);
				auto boundsHit = bounds.intersects(ray.origin, ray.direction);
				if (boundsHit.first)
				{
					Raycast::HitInfo inf;
					inf.distance = boundsHit.second;
					candidates.push_back(std::make_pair(obj, inf));
				}
			}
			else if (terrain != nullptr)
			{
				if (!terrain->getEnabled())
					continue;

				if (checkCastShadows)
				{
					if (terrain->getCastShadows() != castShadows)
						continue;
				}

				AxisAlignedBox bounds = terrain->getBounds(true);
				auto boundsHit = bounds.intersects(ray.origin, ray.direction);
				if (boundsHit.first)
				{
					Raycast::HitInfo inf;
					inf.distance = boundsHit.second;
					candidates.push_back(std::make_pair(obj, inf));
				}
			}
		}

		if (checkBatchedGeometry)
		{
			for (auto& batch : BatchedGeometry::getSingleton()->getBatches())
			{
				if (!layerMask.getLayer(batch->getLayer()))
					continue;

				if (checkLightingStatic)
				{
					if (batch->getLightingStatic() != lightingStatic)
						continue;
				}

				if (checkCastShadows)
				{
					if (batch->getCastShadows() != castShadows)
						continue;
				}

				AxisAlignedBox bounds = batch->getBounds(true);
				auto boundsHit = bounds.intersects(ray.origin, ray.direction);
				if (boundsHit.first)
				{
					Raycast::HitInfo inf;
					inf.distance = boundsHit.second;
					inf.batch = batch;
					candidates.push_back(std::make_pair(nullptr, inf));
				}
			}
		}

		if (checkCSGGeometry)
		{
			for (auto& model : CSGGeometry::getSingleton()->getModels())
			{
				for (auto& subMesh : model->subMeshes)
				{
					if (!layerMask.getLayer(subMesh->getLayer()))
						continue;

					if (checkCastShadows)
					{
						if (subMesh->getCastShadows() != castShadows)
							continue;
					}

					AxisAlignedBox bounds = subMesh->getBounds(true);
					auto boundsHit = bounds.intersects(ray.origin, ray.direction);
					if (boundsHit.first)
					{
						Raycast::HitInfo inf;
						inf.distance = boundsHit.second;
						inf.csgSubMesh = subMesh;
						candidates.push_back(std::make_pair(nullptr, inf));
					}
				}
			}
		}

		std::vector<Raycast::HitInfo> candidates2;

		if (raycastTransientRenderables)
		{
			for (auto it = renderables.begin(); it != renderables.end(); ++it)
			{
				TransientRenderable& trend = *it;

				for (int i = 0; i < trend.triangles.size(); i += 3)
				{
					glm::vec3 p1 = trend.triangles[i];
					glm::vec3 p2 = trend.triangles[i + 1];
					glm::vec3 p3 = trend.triangles[i + 2];

					auto hit = hitTestTriangle(p1, p2, p3, trend.transform, ray);

					if (hit.first)
					{
						hit.second.object = trend.attachedTransform;

						candidates2.push_back(hit.second);
						break;
					}
				}
			}
		}

		for (auto it = candidates.begin(); it != candidates.end(); ++it)
		{
			GameObject* obj = it->first;
			Raycast::HitInfo inf = it->second;

			if (obj != nullptr)
			{
				MeshRenderer* rend = (MeshRenderer*)obj->getComponent(MeshRenderer::COMPONENT_TYPE);
				Terrain* terrain = (Terrain*)obj->getComponent(Terrain::COMPONENT_TYPE);
				
				if (rend != nullptr)
				{
					bool meshHit = hitTestMesh(rend, ray, inf);

					if (meshHit)
					{
						inf.object = obj->getTransform();
						candidates2.push_back(inf);
					}
				}
				else if (terrain != nullptr)
				{
					bool terrainHit = hitTestTerrain(terrain, ray, inf);

					if (terrainHit)
					{
						inf.object = obj->getTransform();
						candidates2.push_back(inf);
					}
				}
			}
			else
			{
				if (inf.batch != nullptr)
				{
					bool batchHit = hitTestBatch(inf.batch, ray, inf);

					if (batchHit)
					{
						inf.object = nullptr;
						candidates2.push_back(inf);
					}
				}
				else if (inf.csgSubMesh != nullptr)
				{
					bool csgHit = hitTestCSG(inf.csgSubMesh, ray, inf);

					if (csgHit)
					{
						inf.object = nullptr;

						if (findCSGBrushes)
						{
							GameObject* obj = Engine::getSingleton()->getGameObject(inf.csgBrushId);
							if (obj != nullptr)
								inf.object = obj->getTransform();
						}

						candidates2.push_back(inf);
					}
				}
			}
		}

		std::sort(candidates2.begin(), candidates2.end(), [=](Raycast::HitInfo& p1, Raycast::HitInfo& p2) -> bool {
			return p1.distance < p2.distance;
		});

		return candidates2;
	}

	std::vector<Raycast::HitInfo> Raycast::executeBoundsOnly(Ray ray)
	{
		std::vector<GameObject*> objects = Engine::getSingleton()->getGameObjects();

		std::vector<Raycast::HitInfo> candidates;

		for (auto it = objects.begin(); it != objects.end(); ++it)
		{
			GameObject* obj = *it;

			if (!obj->getActive())
				continue;

			MeshRenderer* rend = (MeshRenderer*)obj->getComponent(MeshRenderer::COMPONENT_TYPE);
			if (rend != nullptr)
			{
				if (!rend->getEnabled())
					continue;

				AxisAlignedBox bounds = rend->getBounds(true);
				auto boundsHit = bounds.intersects(ray.origin, ray.direction);
				if (boundsHit.first)
				{
					Raycast::HitInfo inf;
					inf.distance = boundsHit.second;
					inf.object = obj->getTransform();
					candidates.push_back(inf);
				}
			}
			else
			{
				Terrain* terrain = (Terrain*)obj->getComponent(Terrain::COMPONENT_TYPE);
				if (terrain != nullptr)
				{
					if (!terrain->getEnabled())
						continue;

					AxisAlignedBox bounds = terrain->getBounds(true);
					auto boundsHit = bounds.intersects(ray.origin, ray.direction);
					if (boundsHit.first)
					{
						Raycast::HitInfo inf;
						inf.distance = boundsHit.second;
						inf.object = obj->getTransform();
						candidates.push_back(inf);
					}
				}
			}
		}

		return candidates;
	}

	bool Raycast::hitTestMesh(MeshRenderer* meshRenderer, Ray ray, Raycast::HitInfo& inf)
	{
		bool ret = false;

		Mesh* mesh = meshRenderer->getMesh();

		if (mesh == nullptr)
			return false;

		glm::mat4x4 mtx = meshRenderer->getGameObject()->getTransform()->getTransformMatrix();
		glm::quat rot = meshRenderer->getGameObject()->getTransform()->getRotation();

		std::pair<float, glm::vec3> minDist = std::make_pair(FLT_MAX, glm::vec3(0, 0, 0));

		for (int i = 0; i < mesh->getSubMeshCount(); ++i)
		{
			SubMesh* subMesh = mesh->getSubMesh(i);
			std::vector<VertexBuffer>& vbuf = subMesh->getVertexBuffer();
			std::vector<uint32_t>& ibuf = subMesh->getIndexBuffer();

			Material* mat = Material::load(Engine::getSingleton()->getAssetsPath(), subMesh->getMaterialName());

			for (uint32_t j = 0; j < ibuf.size(); j += 3)
			{
				uint32_t idx1 = ibuf[j];
				uint32_t idx2 = ibuf[j + 1];
				uint32_t idx3 = ibuf[j + 2];

				VertexBuffer& buf1 = vbuf[idx1];
				VertexBuffer& buf2 = vbuf[idx2];
				VertexBuffer& buf3 = vbuf[idx3];

				glm::vec3 p1 = mtx * glm::vec4(buf1.position, 1.0f);
				glm::vec3 p2 = mtx * glm::vec4(buf2.position, 1.0f);
				glm::vec3 p3 = mtx * glm::vec4(buf3.position, 1.0f);

				// check for a hit against this triangle
				std::pair<bool, float> hit = Mathf::intersects(ray, p1, p2, p3, true, false);

				// if it was a hit check if its the closest
				if (hit.first)
				{
					if (hit.second < minDist.first)
					{
						ret = true;

						//Calculate intersection point
						glm::vec3 D = ray.direction;
						glm::vec3 N = glm::cross(p2 - p1, p3 - p1);
						glm::vec3 X = ray.origin + D * glm::dot(p1 - ray.origin, N) / dot(D, N);

						minDist.first = hit.second;
						minDist.second = X;

						inf.hitNormal = N;

						if (fetchAdditionalInfo)
						{
							inf.hitTrianglePosition[0] = p1;
							inf.hitTrianglePosition[1] = p2;
							inf.hitTrianglePosition[2] = p3;

							glm::vec3 n1 = glm::normalize(rot * buf1.normal);
							glm::vec3 n2 = glm::normalize(rot * buf2.normal);
							glm::vec3 n3 = glm::normalize(rot * buf3.normal);

							inf.hitTriangleNormal[0] = n1;
							inf.hitTriangleNormal[1] = n2;
							inf.hitTriangleNormal[2] = n3;

							inf.hitTriangleTexCoord0[0] = vbuf[idx1].texcoord0;
							inf.hitTriangleTexCoord0[1] = vbuf[idx2].texcoord0;
							inf.hitTriangleTexCoord0[2] = vbuf[idx3].texcoord0;

							inf.hitTriangleTexCoord1[0] = vbuf[idx1].texcoord1;
							inf.hitTriangleTexCoord1[1] = vbuf[idx2].texcoord1;
							inf.hitTriangleTexCoord1[2] = vbuf[idx3].texcoord1;
						}

						inf.material = mat;
						inf.renderer = meshRenderer;
						inf.subMeshIndex = i;
					}
				}
			}
		}

		inf.distance = minDist.first;
		inf.hitPoint = minDist.second;

		return ret;
	}

	bool Raycast::hitTestBatch(BatchedGeometry::Batch* batch, Ray ray, Raycast::HitInfo& inf)
	{
		bool ret = false;

		std::pair<float, glm::vec3> minDist = std::make_pair(FLT_MAX, glm::vec3(0, 0, 0));

		std::vector<VertexBuffer>& vbuf = batch->getVertexBuffer();
		std::vector<uint32_t>& ibuf = batch->getIndexBuffer();

		Material* mat = batch->getMaterial();

		for (uint32_t j = 0; j < ibuf.size(); j += 3)
		{
			uint32_t idx1 = ibuf[j];
			uint32_t idx2 = ibuf[j + 1];
			uint32_t idx3 = ibuf[j + 2];

			VertexBuffer& buf1 = vbuf[idx1];
			VertexBuffer& buf2 = vbuf[idx2];
			VertexBuffer& buf3 = vbuf[idx3];

			// check for a hit against this triangle
			std::pair<bool, float> hit = Mathf::intersects(ray, buf1.position, buf2.position, buf3.position, true, false);

			// if it was a hit check if its the closest
			if (hit.first)
			{
				if (hit.second < minDist.first)
				{
					ret = true;

					//Calculate intersection point
					glm::vec3 D = ray.direction;
					glm::vec3 N = glm::cross(buf2.position - buf1.position, buf3.position - buf1.position);
					glm::vec3 X = ray.origin + D * glm::dot(buf1.position - ray.origin, N) / dot(D, N);

					minDist.first = hit.second;
					minDist.second = X;

					inf.hitNormal = N;

					if (fetchAdditionalInfo)
					{
						inf.hitTrianglePosition[0] = buf1.position;
						inf.hitTrianglePosition[1] = buf2.position;
						inf.hitTrianglePosition[2] = buf3.position;

						inf.hitTriangleNormal[0] = buf1.normal;
						inf.hitTriangleNormal[1] = buf2.normal;
						inf.hitTriangleNormal[2] = buf3.normal;

						inf.hitTriangleTexCoord0[0] = vbuf[idx1].texcoord0;
						inf.hitTriangleTexCoord0[1] = vbuf[idx2].texcoord0;
						inf.hitTriangleTexCoord0[2] = vbuf[idx3].texcoord0;

						inf.hitTriangleTexCoord1[0] = vbuf[idx1].texcoord1;
						inf.hitTriangleTexCoord1[1] = vbuf[idx2].texcoord1;
						inf.hitTriangleTexCoord1[2] = vbuf[idx3].texcoord1;
					}

					inf.material = mat;
				}
			}
		}

		inf.distance = minDist.first;
		inf.hitPoint = minDist.second;

		return ret;
	}

	bool Raycast::hitTestCSG(CSGGeometry::SubMesh* subMesh, Ray ray, Raycast::HitInfo& inf)
	{
		bool ret = false;

		std::pair<float, glm::vec3> minDist = std::make_pair(FLT_MAX, glm::vec3(0, 0, 0));

		std::vector<VertexBuffer>& vbuf = subMesh->getVertexBuffer();
		std::vector<uint32_t>& ibuf = subMesh->getIndexBuffer();
		std::vector<unsigned long long>& idbuf = subMesh->getIdBuffer();

		Material* mat = subMesh->getMaterial();

		for (uint32_t j = 0; j < ibuf.size(); j += 3)
		{
			uint32_t idx1 = ibuf[j];
			uint32_t idx2 = ibuf[j + 1];
			uint32_t idx3 = ibuf[j + 2];

			VertexBuffer& buf1 = vbuf[idx1];
			VertexBuffer& buf2 = vbuf[idx2];
			VertexBuffer& buf3 = vbuf[idx3];

			// check for a hit against this triangle
			std::pair<bool, float> hit = Mathf::intersects(ray, buf1.position, buf2.position, buf3.position, true, false);

			// if it was a hit check if its the closest
			if (hit.first)
			{
				if (hit.second < minDist.first)
				{
					ret = true;

					//Calculate intersection point
					glm::vec3 D = ray.direction;
					glm::vec3 N = glm::cross(buf2.position - buf1.position, buf3.position - buf1.position);
					glm::vec3 X = ray.origin + D * glm::dot(buf1.position - ray.origin, N) / dot(D, N);

					minDist.first = hit.second;
					minDist.second = X;

					inf.hitNormal = N;

					if (fetchAdditionalInfo)
					{
						inf.hitTrianglePosition[0] = buf1.position;
						inf.hitTrianglePosition[1] = buf2.position;
						inf.hitTrianglePosition[2] = buf3.position;

						inf.hitTriangleNormal[0] = buf1.normal;
						inf.hitTriangleNormal[1] = buf2.normal;
						inf.hitTriangleNormal[2] = buf3.normal;

						inf.hitTriangleTexCoord0[0] = vbuf[idx1].texcoord0;
						inf.hitTriangleTexCoord0[1] = vbuf[idx2].texcoord0;
						inf.hitTriangleTexCoord0[2] = vbuf[idx3].texcoord0;

						inf.hitTriangleTexCoord1[0] = vbuf[idx1].texcoord1;
						inf.hitTriangleTexCoord1[1] = vbuf[idx2].texcoord1;
						inf.hitTriangleTexCoord1[2] = vbuf[idx3].texcoord1;
					}

					inf.material = mat;

					if (idx1 < idbuf.size())
						inf.csgBrushId = idbuf[idx1];
				}
			}
		}

		inf.distance = minDist.first;
		inf.hitPoint = minDist.second;

		return ret;
	}

	bool Raycast::hitTestTerrain(Terrain* terrain, Ray ray, Raycast::HitInfo& inf)
	{
		bool ret = false;

		std::pair<float, glm::vec3> minDist = std::make_pair(FLT_MAX, glm::vec3(0, 0, 0));

		Terrain::VertexBuffer* vbuf = terrain->getVertexBuffer();
		uint32_t* ibuf = terrain->getIndexBuffer();
		uint32_t size = terrain->getIndexCount();

		if (terrain->getIsDirty())
			return false;
		if (terrain->getIndexCount() == 0)
			return false;
		if (terrain->getVertexCount() == 0)
			return false;

		glm::mat4x4 mtx = terrain->getGameObject()->getTransform()->getTransformMatrix();

		for (uint32_t j = 0; j < size; j += 3)
		{
			uint32_t idx1 = ibuf[j];
			uint32_t idx2 = ibuf[j + 1];
			uint32_t idx3 = ibuf[j + 2];

			Terrain::VertexBuffer& buf1 = vbuf[idx1];
			Terrain::VertexBuffer& buf2 = vbuf[idx2];
			Terrain::VertexBuffer& buf3 = vbuf[idx3];

			glm::vec3 p1 = mtx * glm::vec4(buf1.position, 1.0f);
			glm::vec3 p2 = mtx * glm::vec4(buf2.position, 1.0f);
			glm::vec3 p3 = mtx * glm::vec4(buf3.position, 1.0f);

			// check for a hit against this triangle
			std::pair<bool, float> hit = Mathf::intersects(ray, p1, p2, p3, true, false);

			// if it was a hit check if its the closest
			if (hit.first)
			{
				if (hit.second < minDist.first)
				{
					ret = true;

					//Calculate intersection point
					glm::vec3 D = ray.direction;
					glm::vec3 N = glm::cross(p2 - p1, p3 - p1);
					glm::vec3 X = ray.origin + D * glm::dot(p1 - ray.origin, N) / dot(D, N);

					minDist.first = hit.second;
					minDist.second = X;

					inf.hitNormal = N;
				}
			}
		}

		inf.distance = minDist.first;
		inf.hitPoint = minDist.second;

		return ret;
	}

	std::pair<bool, Raycast::HitInfo> Raycast::hitTestTriangle(glm::vec3& pp1, glm::vec3& pp2, glm::vec3& pp3, glm::mat4x4& mtx, Ray& ray)
	{
		glm::vec3 p1 = mtx * glm::vec4(pp1, 1.0f);
		glm::vec3 p2 = mtx * glm::vec4(pp2, 1.0f);
		glm::vec3 p3 = mtx * glm::vec4(pp3, 1.0f);

		// check for a hit against this triangle
		std::pair<bool, float> hit = Mathf::intersects(ray, p1, p2, p3, true, true);

		Raycast::HitInfo inf;
		inf.distance = hit.second;

		glm::vec3 D = ray.direction;
		glm::vec3 N = glm::cross(p2 - p1, p3 - p1);
		glm::vec3 X = ray.origin + D * glm::dot(p1 - ray.origin, N) / dot(D, N);

		inf.hitPoint = X;
		inf.hitNormal = N;

		// if it was a hit check if its the closest
		return std::make_pair(hit.first, inf);
	}
}