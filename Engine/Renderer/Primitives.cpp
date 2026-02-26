#include "Primitives.h"

#include "../glm/glm.hpp"
#include "../glm/gtc/type_ptr.hpp"

#include "../Core/Engine.h"
#include "../Math/Mathf.h"
#include "../Renderer/Renderer.h"
#include "../Renderer/VertexLayouts.h"
#include "../Assets/Texture.h"
#include "../Assets/Mesh.h"
#include "../Assets/Material.h"
#include "../Assets/Shader.h"
#include "../Components/Transform.h"
#include "../Components/Camera.h"
#include "../Core/GameObject.h"

namespace GX
{
	std::map<int, Primitives::Sphere::Mesh> Primitives::Sphere::meshes;

	void Primitives::Sphere::build(int size)
	{
		if (meshes[size].indices.size() > 0)
			return;

		float radius = 1.0f;
		int sectorCount = size;
		int stackCount = size / 2;

		float x, y, z, xy;                              // vertex position
		float nx, ny, nz, lengthInv = 1.0f / radius;    // vertex normal
		float s, t;                                     // vertex texCoord

		float sectorStep = 2 * Mathf::PI / sectorCount;
		float stackStep = Mathf::PI / stackCount;
		float sectorAngle, stackAngle;

		for (int i = 0; i <= stackCount; ++i)
		{
			stackAngle = Mathf::PI / 2 - i * stackStep;        // starting from pi/2 to -pi/2
			xy = radius * cosf(stackAngle);             // r * cos(u)
			z = radius * sinf(stackAngle);              // r * sin(u)

			// add (sectorCount+1) vertices per stack
			// the first and last vertices have same position and normal, but different tex coords
			for (int j = 0; j <= sectorCount; ++j)
			{
				sectorAngle = j * sectorStep;           // starting from 0 to 2pi

				// vertex position (x, y, z)
				x = xy * cosf(sectorAngle);             // r * cos(u) * cos(v)
				y = xy * sinf(sectorAngle);             // r * cos(u) * sin(v)
				meshes[size].vertices.push_back(Mathf::toQuaternion(glm::vec3(90, 0, 0)) * glm::vec3(x, y, z));

				// normalized vertex normal (nx, ny, nz)
				nx = x * lengthInv;
				ny = y * lengthInv;
				nz = z * lengthInv;
				meshes[size].normals.push_back(Mathf::toQuaternion(glm::vec3(90, 0, 0)) * glm::vec3(nx, ny, nz));

				// vertex tex coord (s, t) range between [0, 1]
				s = (float)j / (float)sectorCount;
				t = (float)i / (float)stackCount;
				meshes[size].texCoords.push_back(glm::vec2(s, t));
			}
		}

		int k1, k2;
		for (int i = 0; i < stackCount; ++i)
		{
			k1 = i * (sectorCount + 1);     // beginning of current stack
			k2 = k1 + sectorCount + 1;      // beginning of next stack

			for (int j = 0; j < sectorCount; ++j, ++k1, ++k2)
			{
				// 2 triangles per sector excluding first and last stacks
				// k1 => k2 => k1+1
				if (i != 0)
				{
					meshes[size].indices.push_back(k1);
					meshes[size].indices.push_back(k2);
					meshes[size].indices.push_back(k1 + 1);
				}

				// k1+1 => k2 => k2+1
				if (i != (stackCount - 1))
				{
					meshes[size].indices.push_back(k1 + 1);
					meshes[size].indices.push_back(k2);
					meshes[size].indices.push_back(k2 + 1);
				}
			}
		}
	}

	bool Primitives::screenSpaceQuad()
	{
		if (6 == bgfx::getAvailTransientVertexBuffer(6, VertexLayouts::primitiveVertexLayout))
		{
			bgfx::TransientVertexBuffer vb;
			bgfx::allocTransientVertexBuffer(&vb, 6, VertexLayouts::primitiveVertexLayout);
			PosTexCoord0Vertex* vertex = (PosTexCoord0Vertex*)vb.data;

			float zz = 0;

			vertex[0].m_x = 0;
			vertex[0].m_y = 0;
			vertex[0].m_z = zz;
			vertex[0].m_u = 0;
			vertex[0].m_v = 1;

			vertex[1].m_x = 0;
			vertex[1].m_y = 1;
			vertex[1].m_z = zz;
			vertex[1].m_u = 0;
			vertex[1].m_v = 0;

			vertex[2].m_x = 1;
			vertex[2].m_y = 1;
			vertex[2].m_z = zz;
			vertex[2].m_u = 1;
			vertex[2].m_v = 0;

			//

			vertex[3].m_x = 1;
			vertex[3].m_y = 1;
			vertex[3].m_z = zz;
			vertex[3].m_u = 1;
			vertex[3].m_v = 0;

			vertex[4].m_x = 1;
			vertex[4].m_y = 0;
			vertex[4].m_z = zz;
			vertex[4].m_u = 1;
			vertex[4].m_v = 1;

			vertex[5].m_x = 0;
			vertex[5].m_y = 0;
			vertex[5].m_z = zz;
			vertex[5].m_u = 0;
			vertex[5].m_v = 1;

			bgfx::setVertexBuffer(0, &vb);

			return true;
		}

		return false;
	}

	void Primitives::billboard(Transform* trans, Texture* texture, Color color, float scale, int viewId, uint64_t state, bgfx::ProgramHandle program, Camera* camera)
	{
		if (6 == bgfx::getAvailTransientVertexBuffer(6, VertexLayouts::primitiveVertexLayout))
		{
			bgfx::TransientVertexBuffer vb;
			bgfx::allocTransientVertexBuffer(&vb, 6, VertexLayouts::primitiveVertexLayout);
			PosTexCoord0Vertex* vertex = (PosTexCoord0Vertex*)vb.data;

			float zz = 0;
			float sz = 0.5f;

			vertex[0].m_x = -sz;
			vertex[0].m_y = -sz;
			vertex[0].m_z = zz;
			vertex[0].m_u = 0;
			vertex[0].m_v = 1;

			vertex[1].m_x = -sz;
			vertex[1].m_y = sz;
			vertex[1].m_z = zz;
			vertex[1].m_u = 0;
			vertex[1].m_v = 0;

			vertex[2].m_x = sz;
			vertex[2].m_y = sz;
			vertex[2].m_z = zz;
			vertex[2].m_u = -1;
			vertex[2].m_v = 0;

			//

			vertex[3].m_x = sz;
			vertex[3].m_y = sz;
			vertex[3].m_z = zz;
			vertex[3].m_u = -1;
			vertex[3].m_v = 0;

			vertex[4].m_x = sz;
			vertex[4].m_y = -sz;
			vertex[4].m_z = zz;
			vertex[4].m_u = -1;
			vertex[4].m_v = 1;

			vertex[5].m_x = -sz;
			vertex[5].m_y = -sz;
			vertex[5].m_z = zz;
			vertex[5].m_u = 0;
			vertex[5].m_v = 1;

			glm::vec3 position = trans->getPosition();

			Transform* ct = camera->getTransform();
			glm::mat4x4 mtx = glm::identity<glm::mat4x4>();
			mtx = glm::translate(mtx, position);
			mtx = glm::scale(mtx, glm::vec3(scale));
			glm::highp_quat rot = glm::quatLookAt(ct->getForward(), -ct->getUp());
			mtx = mtx * glm::mat4_cast(rot);

			bgfx::setVertexBuffer(0, &vb);
			bgfx::setTransform(glm::value_ptr(mtx), 1);
			bgfx::setState(state);

			glm::vec4 transparent = glm::vec4(0, 0, 0, 0);

			bgfx::setTexture(0, Renderer::getSingleton()->u_albedoMap, texture->getHandle());
			bgfx::setUniform(Renderer::getSingleton()->u_color, color.ptr(), 1);

			TransientRenderable tRend;
			tRend.attachedTransform = trans;
			tRend.transform = mtx;
			tRend.triangles.push_back(glm::vec3(vertex[0].m_x, vertex[0].m_y, vertex[0].m_z));
			tRend.triangles.push_back(glm::vec3(vertex[1].m_x, vertex[1].m_y, vertex[1].m_z));
			tRend.triangles.push_back(glm::vec3(vertex[2].m_x, vertex[2].m_y, vertex[2].m_z));
			tRend.triangles.push_back(glm::vec3(vertex[3].m_x, vertex[3].m_y, vertex[3].m_z));
			tRend.triangles.push_back(glm::vec3(vertex[4].m_x, vertex[4].m_y, vertex[4].m_z));
			tRend.triangles.push_back(glm::vec3(vertex[5].m_x, vertex[5].m_y, vertex[5].m_z));

			Renderer::getSingleton()->addTransientRenderable(tRend);

			bgfx::submit(viewId, program);
		}
	}

	void Primitives::mesh(glm::mat4x4 trans, Mesh* _mesh, Color color, int viewId, uint64_t state, bgfx::ProgramHandle program, Camera* camera)
	{
		for (int i = 0; i < _mesh->getSubMeshCount(); ++i)
		{
			SubMesh* sm = _mesh->getSubMesh(i);

			bgfx::setVertexBuffer(0, sm->getVertexBufferHandle());
			bgfx::setIndexBuffer(sm->getIndexBufferHandle());
			bgfx::setTransform(glm::value_ptr(trans), 1);
			bgfx::setState(state);

			bgfx::setUniform(Renderer::getSingleton()->u_color, color.ptr(), 1);

			bgfx::submit(viewId, program);
		}
	}

	void Primitives::mesh(glm::mat4x4 trans, Mesh* _mesh, Material* material, int viewId, uint64_t state, Camera* camera, std::function<void()> cb)
	{
		for (int i = 0; i < _mesh->getSubMeshCount(); ++i)
		{
			SubMesh* sm = _mesh->getSubMesh(i);
			assert(material != nullptr && "No material passed");

			Shader* shader = material->getShader();
			
			if (shader == nullptr)
				return;

			for (auto& pass : shader->getPasses())
			{
				ProgramVariant* pv = pass->getProgramVariant(material->getDefinesStringHash());
				if (pv == nullptr)
					continue;

				uint64_t passState = state;
				passState = pv->getRenderState(state);

				bgfx::setVertexBuffer(0, sm->getVertexBufferHandle());
				bgfx::setIndexBuffer(sm->getIndexBufferHandle());
				bgfx::setTransform(glm::value_ptr(trans), 1);
				bgfx::setState(passState);

				material->submitUniforms(pv, camera);
				Renderer::getSingleton()->setSystemUniforms(nullptr);

				if (cb != nullptr)
					cb();

				bgfx::submit(viewId, pv->programHandle);
			}
		}
	}

	void Primitives::mesh(glm::mat4x4 trans, Mesh* _mesh, std::vector<Material*> materials, int viewId, uint64_t state, Camera* camera, int renderMode, std::function<void()> cb, int lod)
	{
		RenderMode mode = static_cast<RenderMode>(renderMode);

		for (int i = 0; i < _mesh->getSubMeshCount(); ++i)
		{
			SubMesh* sm = _mesh->getSubMesh(i);

			Material* material = Renderer::getDefaultMaterial();
			Shader* shader = Renderer::getDefaultShader();

			if (i < materials.size())
			{
				if (materials[i] != nullptr)
					material = materials[i];
			}

			if (material->getShader() != nullptr)
				shader = material->getShader();

			if (shader->getRenderMode() != mode)
				continue;

			for (auto& pass : shader->getPasses())
			{
				ProgramVariant* pv = pass->getProgramVariant(material->getDefinesStringHash());

				uint64_t passState = state;
				passState = pv->getRenderState(state);

				bgfx::setVertexBuffer(0, sm->getVertexBufferHandle());
				
				if (lod > -1)
				{
					if (sm->getLodIndexBuffer(lod).size() > 0)
						bgfx::setIndexBuffer(sm->getLodIndexBufferHandle(lod));
					else
						bgfx::setIndexBuffer(sm->getIndexBufferHandle());
				}
				else
					bgfx::setIndexBuffer(sm->getIndexBufferHandle());

				bgfx::setTransform(glm::value_ptr(trans), 1);
				bgfx::setState(passState);

				material->submitUniforms(pv, camera);
				Renderer::getSingleton()->setSystemUniforms(nullptr);

				if (cb != nullptr)
					cb();

				bgfx::submit(viewId, pv->programHandle);
			}
		}
	}

	void Primitives::mesh(glm::mat4x4 trans, std::vector<glm::vec3>& vertices, Color color, int viewId, uint64_t state, bgfx::ProgramHandle program, Camera* camera)
	{
		int avail = bgfx::getAvailTransientVertexBuffer(vertices.size(), VertexLayouts::primitiveVertexLayout);

		if (avail == vertices.size())
		{
			bgfx::TransientVertexBuffer vb;
			bgfx::allocTransientVertexBuffer(&vb, avail, VertexLayouts::primitiveVertexLayout);
			PosTexCoord0Vertex* vertex = (PosTexCoord0Vertex*)vb.data;

			for (int i = 0; i < avail; ++i)
				vertex[i] = { vertices[i].x, vertices[i].y, vertices[i].z, 0, 0 };

			bgfx::setVertexBuffer(0, &vb);

			bgfx::setTransform(glm::value_ptr(trans), 1);
			bgfx::setState(state);

			glm::vec4 transparent = glm::vec4(0, 0, 0, 0);
			bgfx::setTexture(0, Renderer::getSingleton()->u_albedoMap, Texture::getNullTexture()->getHandle());
			bgfx::setUniform(Renderer::getSingleton()->u_color, color.ptr(), 1);

			bgfx::submit(viewId, program);
		}
	}

	void Primitives::box(glm::mat4x4 trans, Color color, int viewId, uint64_t state, bgfx::ProgramHandle program, Camera* camera)
	{
		if (8 == bgfx::getAvailTransientVertexBuffer(8, VertexLayouts::primitiveVertexLayout))
		{
			bgfx::TransientVertexBuffer vb;
			bgfx::TransientIndexBuffer ib;
			bgfx::allocTransientVertexBuffer(&vb, 8, VertexLayouts::primitiveVertexLayout);
			bgfx::allocTransientIndexBuffer(&ib, 24);
			PosTexCoord0Vertex* vertex = (PosTexCoord0Vertex*)vb.data;
			uint16_t* index = (uint16_t*)ib.data;

			vertex[0] = { -1.0f,  1.0f,  1.0f, 0, 0 };
			vertex[1] = { 1.0f,  1.0f,  1.0f, 0, 0 };
			vertex[2] = { -1.0f, -1.0f,  1.0f, 0, 0 };
			vertex[3] = { 1.0f, -1.0f,  1.0f, 0, 0 };
			vertex[4] = { -1.0f,  1.0f, -1.0f, 0, 0 };
			vertex[5] = { 1.0f,  1.0f, -1.0f, 0, 0 };
			vertex[6] = { -1.0f, -1.0f, -1.0f, 0, 0 };
			vertex[7] = { 1.0f, -1.0f, -1.0f, 0, 0 };

			index[0] = 0;
			index[1] = 1;
			index[2] = 0;
			index[3] = 2;
			index[4] = 0;
			index[5] = 4;
			index[6] = 1;
			index[7] = 3;
			index[8] = 1;
			index[9] = 5;
			index[10] = 2;
			index[11] = 3;
			index[12] = 2;
			index[13] = 6;
			index[14] = 3;
			index[15] = 7;
			index[16] = 4;
			index[17] = 5;
			index[18] = 4;
			index[19] = 6;
			index[20] = 5;
			index[21] = 7;
			index[22] = 6;
			index[23] = 7;

			bgfx::setVertexBuffer(0, &vb);
			bgfx::setIndexBuffer(&ib);
			
			bgfx::setTransform(glm::value_ptr(trans), 1);
			bgfx::setState(state);

			bgfx::setUniform(Renderer::getSingleton()->u_color, color.ptr(), 1);
			bgfx::setTexture(0, Renderer::getSingleton()->u_albedoMap, Texture::getNullTexture()->getHandle());
			bgfx::setUniform(Renderer::getSingleton()->u_color, color.ptr(), 1);

			bgfx::submit(viewId, program);
		}
	}

	void Primitives::cube()
	{
		if (24 == bgfx::getAvailTransientVertexBuffer(24, VertexLayouts::primitiveVertexLayout))
		{
			bgfx::TransientVertexBuffer vb;
			bgfx::TransientIndexBuffer ib;
			bgfx::allocTransientVertexBuffer(&vb, 24, VertexLayouts::primitiveVertexLayout);
			bgfx::allocTransientIndexBuffer(&ib, 36);
			PosTexCoord0Vertex* vertex = (PosTexCoord0Vertex*)vb.data;
			uint16_t* index = (uint16_t*)ib.data;

			vertex[0] = { -1.0f,  1.0f,  1.0f, 0, 0 };
			vertex[1] = { 1.0f,  1.0f,  1.0f, 1, 0 };
			vertex[2] = { -1.0f, -1.0f,  1.0f, 0, 1 };
			vertex[3] = { 1.0f, -1.0f, 1.0f, 1, 1 };

			vertex[4] = { -1.0f, 1.0f, -1.0f, 0, 0 };
			vertex[5] = { 1.0f, 1.0f, -1.0f, 1, 0 };
			vertex[6] = { -1.0f, -1.0f, -1.0f, 0, 1 };
			vertex[7] = { 1.0f, -1.0f, -1.0f, 1, 1 };

			//Top
			vertex[8] = { -1.0f, 1.0f, 1.0f, 0, 0 };
			vertex[9] = { 1.0f, 1.0f, 1.0f, 1, 0 };
			vertex[10] = { -1.0f, 1.0f, -1.0f, 0, 1 };
			vertex[11] = { 1.0f, 1.0f, -1.0f, 1, 1 };

			//Bottom
			vertex[12] = { -1.0f, -1.0f, 1.0f, 0, 0 };
			vertex[13] = { 1.0f, -1.0f, 1.0f, 1, 0 };
			vertex[14] = { -1.0f, -1.0f, -1.0f, 0, 1 };
			vertex[15] = { 1.0f, -1.0f, -1.0f, 1, 1 };

			vertex[16] = { 1.0f, -1.0f, 1.0f, 0, 0 };
			vertex[17] = { 1.0f, 1.0f, 1.0f, 1, 0 };
			vertex[18] = { 1.0f, -1.0f, -1.0f, 0, 1 };
			vertex[19] = { 1.0f, 1.0f, -1.0f, 1, 1 };

			vertex[20] = { -1.0f, -1.0f, 1.0f, 0, 0 };
			vertex[21] = { -1.0f, 1.0f, 1.0f, 1, 0 };
			vertex[22] = { -1.0f, -1.0f, -1.0f, 0, 1 };
			vertex[23] = { -1.0f, 1.0f, -1.0f, 1, 1 };

			index[0] = 0;
			index[1] = 2;
			index[2] = 1;
			index[3] = 1;
			index[4] = 2;
			index[5] = 3;

			index[6] = 4;
			index[7] = 5;
			index[8] = 6;
			index[9] = 5;
			index[10] = 7;
			index[11] = 6;

			//Top
			index[12] = 11;
			index[13] = 10;
			index[14] = 9;
			index[15] = 9;
			index[16] = 10;
			index[17] = 8;

			//Bottom
			index[18] = 14;
			index[19] = 15;
			index[20] = 13;
			index[21] = 14;
			index[22] = 13;
			index[23] = 12;

			index[24] = 16;
			index[25] = 18;
			index[26] = 17;
			index[27] = 17;
			index[28] = 18;
			index[29] = 19;

			index[30] = 20;
			index[31] = 21;
			index[32] = 22;
			index[33] = 21;
			index[34] = 23;
			index[35] = 22;

			bgfx::setVertexBuffer(0, &vb);
			bgfx::setIndexBuffer(&ib);
		}
	}

	void Primitives::skybox(glm::mat4x4 trans, int viewId, uint64_t state, bgfx::ProgramHandle program, Camera* camera)
	{
		if (24 == bgfx::getAvailTransientVertexBuffer(24, VertexLayouts::primitiveVertexLayout))
		{
			bgfx::TransientVertexBuffer vb;
			bgfx::TransientIndexBuffer ib;
			bgfx::allocTransientVertexBuffer(&vb, 24, VertexLayouts::primitiveVertexLayout);
			bgfx::allocTransientIndexBuffer(&ib, 36);
			PosTexCoord0Vertex* vertex = (PosTexCoord0Vertex*)vb.data;
			uint16_t* index = (uint16_t*)ib.data;

			vertex[0] = { -1.0f,  1.0f,  1.0f, 0, 0 };
			vertex[1] = { 1.0f,  1.0f,  1.0f, 0x7fff, 0 };
			vertex[2] = { -1.0f, -1.0f,  1.0f, 0, 0x7fff };
			vertex[3] = { 1.0f, -1.0f, 1.0f, 0x7fff, 0x7fff };

			vertex[4] = { -1.0f, 1.0f, -1.0f, 0, 0 };
			vertex[5] = { 1.0f, 1.0f, -1.0f, 0x7fff, 0 };
			vertex[6] = { -1.0f, -1.0f, -1.0f, 0, 0x7fff };
			vertex[7] = { 1.0f, -1.0f, -1.0f, 0x7fff, 0x7fff };
			
			//Top
			vertex[8] = { -1.0f, 1.0f, 1.0f, 0, 0 };
			vertex[9] = { 1.0f, 1.0f, 1.0f, 0x7fff, 0 };
			vertex[10] = { -1.0f, 1.0f, -1.0f, 0, 0x7fff };
			vertex[11] = { 1.0f, 1.0f, -1.0f, 0x7fff, 0x7fff };
			
			//Bottom
			vertex[12] = { -1.0f, -1.0f, 1.0f, 0, 0 };
			vertex[13] = { 1.0f, -1.0f, 1.0f, 0x7fff, 0 };
			vertex[14] = { -1.0f, -1.0f, -1.0f, 0, 0x7fff };
			vertex[15] = { 1.0f, -1.0f, -1.0f, 0x7fff, 0x7fff };

			vertex[16] = { 1.0f, -1.0f, 1.0f, 0, 0 };
			vertex[17] = { 1.0f, 1.0f, 1.0f, 0x7fff, 0 };
			vertex[18] = { 1.0f, -1.0f, -1.0f, 0, 0x7fff };
			vertex[19] = { 1.0f, 1.0f, -1.0f, 0x7fff, 0x7fff };

			vertex[20] = { -1.0f, -1.0f, 1.0f, 0, 0 };
			vertex[21] = { -1.0f, 1.0f, 1.0f, 0x7fff, 0 };
			vertex[22] = { -1.0f, -1.0f, -1.0f, 0, 0x7fff };
			vertex[23] = { -1.0f, 1.0f, -1.0f, 0x7fff, 0x7fff };

			index[0] = 0;
			index[1] = 2;
			index[2] = 1;
			index[3] = 1;
			index[4] = 2;
			index[5] = 3;

			index[6] = 4;
			index[7] = 5;
			index[8] = 6;
			index[9] = 5;
			index[10] = 7;
			index[11] = 6;

			//Top
			index[12] = 8;
			index[13] = 10;
			index[14] = 9;
			index[15] = 9;
			index[16] = 10;
			index[17] = 11;

			//Bottom
			index[18] = 12;
			index[19] = 13;
			index[20] = 14;
			index[21] = 13;
			index[22] = 15;
			index[23] = 14;

			index[24] = 16;
			index[25] = 18;
			index[26] = 17;
			index[27] = 17;
			index[28] = 18;
			index[29] = 19;

			index[30] = 20;
			index[31] = 21;
			index[32] = 22;
			index[33] = 21;
			index[34] = 23;
			index[35] = 22;

			bgfx::setVertexBuffer(0, &vb);
			bgfx::setIndexBuffer(&ib);

			bgfx::setTransform(glm::value_ptr(trans), 1);
			bgfx::setState(state);

			bgfx::submit(viewId, program);
		}
	}

	void Primitives::sphere(const glm::mat4x4& trans, int size, int viewId, uint64_t state, bgfx::ProgramHandle program, Camera* camera)
	{
		if (Sphere::meshes[size].indices.size() == 0)
			Sphere::build(size);

		int vertSize = Sphere::meshes[size].vertices.size();
		int indSize = Sphere::meshes[size].indices.size();

		if (vertSize == bgfx::getAvailTransientVertexBuffer(vertSize, VertexLayouts::primitiveVertexLayout))
		{
			if (indSize == bgfx::getAvailTransientIndexBuffer(indSize))
			{
				bgfx::TransientVertexBuffer vb;
				bgfx::TransientIndexBuffer ib;
				bgfx::allocTransientVertexBuffer(&vb, vertSize, VertexLayouts::primitiveVertexLayout);
				bgfx::allocTransientIndexBuffer(&ib, indSize);
				PosTexCoord0Vertex* vertex = (PosTexCoord0Vertex*)vb.data;
				uint16_t* index = (uint16_t*)ib.data;
				
				for (int i = 0; i < vertSize; ++i)
					vertex[i] = {
					Sphere::meshes[size].vertices[i].x, Sphere::meshes[size].vertices[i].y, Sphere::meshes[size].vertices[i].z,
					Sphere::meshes[size].texCoords[i].x, Sphere::meshes[size].texCoords[i].y
				};

				for (int i = 0; i < indSize; ++i)
					index[i] = Sphere::meshes[size].indices[i];

				bgfx::setVertexBuffer(0, &vb);
				bgfx::setIndexBuffer(&ib);

				bgfx::setTransform(glm::value_ptr(trans), 1);
				bgfx::setState(state);

				bgfx::submit(viewId, program);
			}
		}
	}

	void Primitives::circle(glm::mat4x4 trans, Color color, int viewId, uint64_t state, bgfx::ProgramHandle program, Camera* camera)
	{
		float accuracy = 35;
		int vertsCount = 0;
		for (float theta = 0; theta <= 2 * Mathf::PI; theta += Mathf::PI / accuracy)
			vertsCount += 2;

		if (vertsCount == bgfx::getAvailTransientVertexBuffer(vertsCount, VertexLayouts::primitiveVertexLayout))
		{
			bgfx::TransientVertexBuffer vb;
			bgfx::allocTransientVertexBuffer(&vb, vertsCount, VertexLayouts::primitiveVertexLayout);
			PosTexCoord0Vertex* vertex = (PosTexCoord0Vertex*)vb.data;

			int idx = 0;
			for (float theta = 0; theta <= 2 * Mathf::PI; theta += Mathf::PI / accuracy)
			{
				vertex[idx] = { /*radius * */cosf(theta), 0, /*radius * */sinf(theta), 0, 0 };
				vertex[idx + 1] = { /*radius * */cosf(theta - Mathf::PI / accuracy), 0, /*radius * */sinf(theta - Mathf::PI / accuracy), 0, 0 };

				idx += 2;
			}

			bgfx::setVertexBuffer(0, &vb);

			bgfx::setTransform(glm::value_ptr(trans), 1);
			bgfx::setState(state/* | BGFX_STATE_LINEAA*/);

			glm::vec4 transparent = glm::vec4(0, 0, 0, 0);
			bgfx::setUniform(Renderer::getSingleton()->u_color, color.ptr(), 1);
			bgfx::setTexture(0, Renderer::getSingleton()->u_albedoMap, Texture::getNullTexture()->getHandle());
			bgfx::setUniform(Renderer::getSingleton()->u_color, color.ptr(), 1);

			bgfx::submit(viewId, program);
		}
	}

	void Primitives::circledSphere(glm::mat4x4 trans, Color color, int viewId, uint64_t state, bgfx::ProgramHandle program, Camera* camera)
	{
		float accuracy = 35;
		int vertsCount = 0;
		for (float theta = 0; theta <= 2 * Mathf::PI; theta += Mathf::PI / accuracy)
			vertsCount += 6;

		if (vertsCount == bgfx::getAvailTransientVertexBuffer(vertsCount, VertexLayouts::primitiveVertexLayout))
		{
			bgfx::TransientVertexBuffer vb;
			bgfx::allocTransientVertexBuffer(&vb, vertsCount, VertexLayouts::primitiveVertexLayout);
			PosTexCoord0Vertex* vertex = (PosTexCoord0Vertex*)vb.data;

			int idx = 0;
			for (float theta = 0; theta <= 2 * Mathf::PI; theta += Mathf::PI / accuracy)
			{
				vertex[idx] = { /*radius * */cosf(theta), 0, /*radius * */sinf(theta), 0, 0 };
				vertex[idx + 1] = { /*radius * */cosf(theta - Mathf::PI / accuracy), 0, /*radius * */sinf(theta - Mathf::PI / accuracy), 0, 0 };

				vertex[idx + 2] = { 0, /*radius * */cosf(theta), /*radius * */sinf(theta), 0, 0 };
				vertex[idx + 3] = { 0, /*radius * */cosf(theta - Mathf::PI / accuracy), /*radius * */sinf(theta - Mathf::PI / accuracy), 0, 0 };

				vertex[idx + 4] = { /*radius * */cosf(theta), /*radius * */sinf(theta), 0, 0, 0 };
				vertex[idx + 5] = { /*radius * */cosf(theta - Mathf::PI / accuracy), /*radius * */sinf(theta - Mathf::PI / accuracy), 0, 0, 0 };

				idx += 6;
			}

			bgfx::setVertexBuffer(0, &vb);

			bgfx::setTransform(glm::value_ptr(trans), 1);
			bgfx::setState(state/* | BGFX_STATE_LINEAA*/);

			glm::vec4 transparent = glm::vec4(0, 0, 0, 0);
			bgfx::setUniform(Renderer::getSingleton()->u_color, color.ptr(), 1);
			bgfx::setTexture(0, Renderer::getSingleton()->u_albedoMap, Texture::getNullTexture()->getHandle());
			bgfx::setUniform(Renderer::getSingleton()->u_color, color.ptr(), 1);

			bgfx::submit(viewId, program);
		}
	}

	void Primitives::circledCapsule(glm::mat4x4 trans, float height, float radius, Color color, int viewId, uint64_t state, bgfx::ProgramHandle program, Camera* camera)
	{
		float accuracy = 35;
		int vertsCount = 8;
		for (float theta = 0; theta <= 2 * Mathf::PI; theta += Mathf::PI / accuracy)
			vertsCount += 4;
		for (float theta = Mathf::PI / accuracy; theta <= Mathf::PI + Mathf::PI / accuracy; theta += Mathf::PI / accuracy)
			vertsCount += 8;

		if (vertsCount == bgfx::getAvailTransientVertexBuffer(vertsCount, VertexLayouts::primitiveVertexLayout))
		{
			float _radius = radius;
			if (_radius < 0) _radius = 0;

			bgfx::TransientVertexBuffer vb;
			bgfx::allocTransientVertexBuffer(&vb, vertsCount, VertexLayouts::primitiveVertexLayout);
			PosTexCoord0Vertex* vertex = (PosTexCoord0Vertex*)vb.data;

			float offsetH = height / 2 - _radius;
			if (offsetH < 0) offsetH = 0;

			//Top circle
			int idx = 0;
			for (float theta = 0; theta <= 2 * Mathf::PI; theta += Mathf::PI / accuracy)
			{
				vertex[idx] = { _radius * cosf(theta), _radius * sinf(theta), 0 + offsetH, 0, 0 };
				vertex[idx + 1] = { _radius * cosf(theta - Mathf::PI / accuracy), _radius * sinf(theta - Mathf::PI / accuracy), 0 + offsetH, 0, 0 };

				idx += 2;
			}

			//Top hemispheres
			for (float theta = Mathf::PI / accuracy; theta <= Mathf::PI + Mathf::PI / accuracy; theta += Mathf::PI / accuracy)
			{
				vertex[idx] = { _radius * cosf(theta), 0, _radius * sinf(theta) + offsetH, 0, 0 };
				vertex[idx + 1] = { _radius * cosf(theta - Mathf::PI / accuracy), 0, _radius * sinf(theta - Mathf::PI / accuracy) + offsetH, 0, 0 };

				vertex[idx + 2] = { 0, _radius * cosf(theta), _radius * sinf(theta) + offsetH, 0, 0 };
				vertex[idx + 3] = { 0, _radius * cosf(theta - Mathf::PI / accuracy), _radius * sinf(theta - Mathf::PI / accuracy) + offsetH, 0, 0 };

				idx += 4;
			}

			//Bottom circle
			for (float theta = 0; theta <= 2 * Mathf::PI; theta += Mathf::PI / accuracy)
			{
				vertex[idx] = { _radius * cosf(theta), _radius * sinf(theta), 0 - offsetH, 0, 0 };
				vertex[idx + 1] = { _radius * cosf(theta - Mathf::PI / accuracy), _radius * sinf(theta - Mathf::PI / accuracy), 0 - offsetH, 0, 0 };

				idx += 2;
			}

			//Bottom hemispheres
			for (float theta = Mathf::PI + Mathf::PI / accuracy; theta <= Mathf::PI * 2.0 + Mathf::PI / accuracy; theta += Mathf::PI / accuracy)
			{
				vertex[idx] = { _radius * cosf(theta), 0, _radius * sinf(theta) - offsetH, 0, 0 };
				vertex[idx + 1] = { _radius * cosf(theta - Mathf::PI / accuracy), 0, _radius * sinf(theta - Mathf::PI / accuracy) - offsetH, 0, 0 };

				vertex[idx + 2] = { 0, _radius * cosf(theta), _radius * sinf(theta) - offsetH, 0, 0 };
				vertex[idx + 3] = { 0, _radius * cosf(theta - Mathf::PI / accuracy), _radius * sinf(theta - Mathf::PI / accuracy) - offsetH, 0, 0 };

				idx += 4;
			}

			//Cylinder lines
			vertex[idx] = { _radius, 0, offsetH, 0, 0 };
			vertex[idx + 1] = { _radius, 0, -offsetH, 0, 0 };

			vertex[idx + 2] = { -_radius, 0, offsetH, 0, 0 };
			vertex[idx + 3] = { -_radius, 0, -offsetH, 0, 0 };

			vertex[idx + 4] = { 0, _radius, offsetH, 0, 0 };
			vertex[idx + 5] = { 0, _radius, -offsetH, 0, 0 };

			vertex[idx + 6] = { 0, -_radius, offsetH, 0, 0 };
			vertex[idx + 7] = { 0, -_radius, -offsetH, 0, 0 };

			bgfx::setVertexBuffer(0, &vb);

			bgfx::setTransform(glm::value_ptr(trans * glm::mat4_cast(Mathf::toQuaternion(0.0f, 0.0f, 90.0f))), 1);
			bgfx::setState(state/* | BGFX_STATE_LINEAA*/);

			glm::vec4 transparent = glm::vec4(0, 0, 0, 0);
			bgfx::setUniform(Renderer::getSingleton()->u_color, color.ptr(), 1);
			bgfx::setTexture(0, Renderer::getSingleton()->u_albedoMap, Texture::getNullTexture()->getHandle());
			bgfx::setUniform(Renderer::getSingleton()->u_color, color.ptr(), 1);

			bgfx::submit(viewId, program);
		}
	}

	void Primitives::circledCone(glm::mat4x4 trans, float range, float radius, Color color, int viewId, uint64_t state, bgfx::ProgramHandle program, Camera* camera)
	{
		float accuracy = 35;
		int vertsCount = 8;
		for (float theta = 0; theta <= 2 * Mathf::PI; theta += Mathf::PI / accuracy)
			vertsCount += 2;

		if (vertsCount == bgfx::getAvailTransientVertexBuffer(vertsCount, VertexLayouts::primitiveVertexLayout))
		{
			bgfx::TransientVertexBuffer vb;
			bgfx::allocTransientVertexBuffer(&vb, vertsCount, VertexLayouts::primitiveVertexLayout);
			PosTexCoord0Vertex* vertex = (PosTexCoord0Vertex*)vb.data;

			int idx = 0;
			for (float theta = 0; theta <= 2 * Mathf::PI; theta += Mathf::PI / accuracy)
			{
				vertex[idx] = { radius * cosf(theta), radius * sinf(theta), -range, 0, 0 };
				vertex[idx + 1] = { radius * cosf(theta - Mathf::PI / accuracy), radius * sinf(theta - Mathf::PI / accuracy), -range, 0, 0 };

				idx += 2;
			}

			float theta = -Mathf::PI / 2;
			vertex[idx] = { 0, 0, 0, 0, 0 };
			vertex[idx + 1] = { radius * cosf(theta), radius * sinf(theta), -range, 0, 0 };

			theta = 0;
			vertex[idx + 2] = { 0, 0, 0, 0, 0 };
			vertex[idx + 3] = { radius * cosf(theta), radius * sinf(theta), -range, 0, 0 };

			theta = Mathf::PI / 2;
			vertex[idx + 4] = { 0, 0, 0, 0, 0 };
			vertex[idx + 5] = { radius * cosf(theta), radius * sinf(theta), -range, 0, 0 };

			theta = Mathf::PI;
			vertex[idx + 6] = { 0, 0, 0, 0, 0 };
			vertex[idx + 7] = { radius * cosf(theta), radius * sinf(theta), -range, 0, 0 };

			bgfx::setVertexBuffer(0, &vb);

			bgfx::setTransform(glm::value_ptr(trans * glm::mat4_cast(Mathf::toQuaternion(0.0f, 180.0f, 0.0f))), 1);
			bgfx::setState(state/* | BGFX_STATE_LINEAA*/);

			glm::vec4 transparent = glm::vec4(0, 0, 0, 0);
			bgfx::setUniform(Renderer::getSingleton()->u_color, color.ptr(), 1);
			bgfx::setTexture(0, Renderer::getSingleton()->u_albedoMap, Texture::getNullTexture()->getHandle());
			bgfx::setUniform(Renderer::getSingleton()->u_color, color.ptr(), 1);

			bgfx::submit(viewId, program);
		}
	}

	void Primitives::circledRays(glm::mat4x4 trans, float range, float radius, Color color, int viewId, uint64_t state, bgfx::ProgramHandle program, Camera* camera)
	{
		float accuracy = 16;
		int vertsCount = 16;
		for (float theta = 0; theta <= 2 * Mathf::PI; theta += Mathf::PI / accuracy)
			vertsCount += 2;

		if (vertsCount == bgfx::getAvailTransientVertexBuffer(vertsCount, VertexLayouts::primitiveVertexLayout))
		{
			bgfx::TransientVertexBuffer vb;
			bgfx::allocTransientVertexBuffer(&vb, vertsCount, VertexLayouts::primitiveVertexLayout);
			PosTexCoord0Vertex* vertex = (PosTexCoord0Vertex*)vb.data;

			int idx = 0;
			for (float theta = 0; theta <= 2 * Mathf::PI; theta += Mathf::PI / accuracy)
			{
				vertex[idx] = { radius * cosf(theta), radius * sinf(theta), 0.0f, 0, 0 };
				vertex[idx + 1] = { radius * cosf(theta - Mathf::PI / accuracy), radius * sinf(theta - Mathf::PI / accuracy), 0.0f, 0, 0 };

				idx += 2;
			}

			float theta = -Mathf::PI / 2;
			vertex[idx] = { radius * cosf(theta), radius * sinf(theta), 0, 0, 0 };
			vertex[idx + 1] = { radius * cosf(theta), radius * sinf(theta), -range, 0, 0 };

			theta = 0;
			vertex[idx + 2] = { radius * cosf(theta), radius * sinf(theta), 0, 0, 0 };
			vertex[idx + 3] = { radius * cosf(theta), radius * sinf(theta), -range, 0, 0 };

			theta = Mathf::PI / 2;
			vertex[idx + 4] = { radius * cosf(theta), radius * sinf(theta), 0, 0, 0 };
			vertex[idx + 5] = { radius * cosf(theta), radius * sinf(theta), -range, 0, 0 };

			theta = Mathf::PI;
			vertex[idx + 6] = { radius * cosf(theta), radius * sinf(theta), 0, 0, 0 };
			vertex[idx + 7] = { radius * cosf(theta), radius * sinf(theta), -range, 0, 0 };

			/////////////////

			theta = -Mathf::PI / 4;
			vertex[idx + 8] = { radius * cosf(theta), radius * sinf(theta), 0, 0, 0 };
			vertex[idx + 9] = { radius * cosf(theta), radius * sinf(theta), -range, 0, 0 };

			theta = Mathf::PI / 4;
			vertex[idx + 10] = { radius * cosf(theta), radius * sinf(theta), 0, 0, 0 };
			vertex[idx + 11] = { radius * cosf(theta), radius * sinf(theta), -range, 0, 0 };

			theta = Mathf::PI / 2 + Mathf::PI / 4;
			vertex[idx + 12] = { radius * cosf(theta), radius * sinf(theta), 0, 0, 0 };
			vertex[idx + 13] = { radius * cosf(theta), radius * sinf(theta), -range, 0, 0 };

			theta = -Mathf::PI / 2 - Mathf::PI / 4;
			vertex[idx + 14] = { radius * cosf(theta), radius * sinf(theta), 0, 0, 0 };
			vertex[idx + 15] = { radius * cosf(theta), radius * sinf(theta), -range, 0, 0 };

			bgfx::setVertexBuffer(0, &vb);

			bgfx::setTransform(glm::value_ptr(trans * glm::mat4_cast(Mathf::toQuaternion(0.0f, 180.0f, 0.0f))), 1);
			bgfx::setState(state/* | BGFX_STATE_LINEAA*/);

			glm::vec4 transparent = glm::vec4(0, 0, 0, 0);
			bgfx::setUniform(Renderer::getSingleton()->u_color, color.ptr(), 1);
			bgfx::setTexture(0, Renderer::getSingleton()->u_albedoMap, Texture::getNullTexture()->getHandle());
			bgfx::setUniform(Renderer::getSingleton()->u_color, color.ptr(), 1);

			bgfx::submit(viewId, program);
		}
	}

	void Primitives::cameraFrustum(glm::mat4x4 trans, Camera* frustum, Color color, int viewId, uint64_t state, bgfx::ProgramHandle program, Camera* camera)
	{
		int vertsCount = 24;

		if (vertsCount == bgfx::getAvailTransientVertexBuffer(vertsCount, VertexLayouts::primitiveVertexLayout))
		{
			bgfx::TransientVertexBuffer vb;
			bgfx::allocTransientVertexBuffer(&vb, vertsCount, VertexLayouts::primitiveVertexLayout);
			PosTexCoord0Vertex* vertex = (PosTexCoord0Vertex*)vb.data;

			float ncd = frustum->getNear();
			float fcd = frustum->getFar();

			glm::vec3 camForward, camRight, camUp;
			camForward = glm::vec3(0, 0, 1);
			camRight = glm::vec3(1, 0, 0);
			camUp = glm::vec3(0, 1, 0);

			glm::vec3 nearCenter = -camForward * ncd;
			glm::vec3 farCenter = -camForward * fcd;

			float w = Renderer::getSingleton()->getSingleton()->getWidth();
			float h = Renderer::getSingleton()->getSingleton()->getHeight();
			float viewRatio = w / h;

			float nearHeight = 2 * tan(frustum->getFOVy() * Mathf::fDeg2Rad / 2) * ncd;
			float farHeight = 2 * tan(frustum->getFOVy() * Mathf::fDeg2Rad / 2) * fcd;
			float nearWidth = nearHeight * viewRatio;
			float farWidth = farHeight * viewRatio;

			glm::vec3 farTopLeft = farCenter + camUp * (farHeight * 0.5f) - camRight * (farWidth * 0.5f);
			glm::vec3 farTopRight = farCenter + camUp * (farHeight * 0.5f) + camRight * (farWidth * 0.5f);
			glm::vec3 farBottomLeft = farCenter - camUp * (farHeight * 0.5f) - camRight * (farWidth * 0.5f);
			glm::vec3 farBottomRight = farCenter - camUp * (farHeight * 0.5f) + camRight * (farWidth * 0.5f);

			glm::vec3 nearTopLeft = nearCenter + camUp * (nearHeight * 0.5f) - camRight * (nearWidth * 0.5f);
			glm::vec3 nearTopRight = nearCenter + camUp * (nearHeight * 0.5f) + camRight * (nearWidth * 0.5f);
			glm::vec3 nearBottomLeft = nearCenter - camUp * (nearHeight * 0.5f) - camRight * (nearWidth * 0.5f);
			glm::vec3 nearBottomRight = nearCenter - camUp * (nearHeight * 0.5f) + camRight * (nearWidth * 0.5f);

			//Near frustum plane
			vertex[0] = { nearBottomLeft.x, nearBottomLeft.y, nearBottomLeft.z, 0, 0 };
			vertex[1] = { nearTopLeft.x, nearTopLeft.y, nearTopLeft.z, 0, 0 };
			vertex[2] = { nearTopLeft.x, nearTopLeft.y, nearTopLeft.z, 0, 0 };
			vertex[3] = { nearTopRight.x, nearTopRight.y, nearTopRight.z, 0, 0 };
			vertex[4] = { nearTopRight.x, nearTopRight.y, nearTopRight.z, 0, 0 };
			vertex[5] = { nearBottomRight.x, nearBottomRight.y, nearBottomRight.z, 0, 0 };
			vertex[6] = { nearBottomRight.x, nearBottomRight.y, nearBottomRight.z, 0, 0 };
			vertex[7] = { nearBottomLeft.x, nearBottomLeft.y, nearBottomLeft.z, 0, 0 };

			//Far frustum plane
			vertex[8] = { farBottomLeft.x, farBottomLeft.y, farBottomLeft.z, 0, 0 };
			vertex[9] = { farTopLeft.x, farTopLeft.y, farTopLeft.z, 0, 0 };
			vertex[10] = { farTopLeft.x, farTopLeft.y, farTopLeft.z, 0, 0 };
			vertex[11] = { farTopRight.x, farTopRight.y, farTopRight.z, 0, 0 };
			vertex[12] = { farTopRight.x, farTopRight.y, farTopRight.z, 0, 0 };
			vertex[13] = { farBottomRight.x, farBottomRight.y, farBottomRight.z, 0, 0 };
			vertex[14] = { farBottomRight.x, farBottomRight.y, farBottomRight.z, 0, 0 };
			vertex[15] = { farBottomLeft.x, farBottomLeft.y, farBottomLeft.z, 0, 0 };

			//Corner lines
			vertex[16] = { nearBottomLeft.x, nearBottomLeft.y, nearBottomLeft.z, 0, 0 };
			vertex[17] = { farBottomLeft.x, farBottomLeft.y, farBottomLeft.z, 0, 0 };
			vertex[18] = { nearTopLeft.x, nearTopLeft.y, nearTopLeft.z, 0, 0 };
			vertex[19] = { farTopLeft.x, farTopLeft.y, farTopLeft.z, 0, 0 };
			vertex[20] = { nearBottomRight.x, nearBottomRight.y, nearBottomRight.z, 0, 0 };
			vertex[21] = { farBottomRight.x, farBottomRight.y, farBottomRight.z, 0, 0 };
			vertex[22] = { nearTopRight.x, nearTopRight.y, nearTopRight.z, 0, 0 };
			vertex[23] = { farTopRight.x, farTopRight.y, farTopRight.z, 0, 0 };

			bgfx::setVertexBuffer(0, &vb);

			bgfx::setTransform(glm::value_ptr(trans * glm::mat4_cast(Mathf::toQuaternion(0.0f, 180.0f, 0.0f))), 1);
			bgfx::setState(state/* | BGFX_STATE_LINEAA*/);

			glm::vec4 transparent = glm::vec4(0, 0, 0, 0);
			bgfx::setUniform(Renderer::getSingleton()->u_color, color.ptr(), 1);
			bgfx::setTexture(0, Renderer::getSingleton()->u_albedoMap, Texture::getNullTexture()->getHandle());
			bgfx::setUniform(Renderer::getSingleton()->u_color, color.ptr(), 1);

			bgfx::submit(viewId, program);
		}
	}

	void Primitives::point(glm::mat4x4 trans, float size, Color color, int viewId, uint64_t state, bgfx::ProgramHandle program, Camera* camera)
	{
		int vertsCount = 1;

		if (vertsCount == bgfx::getAvailTransientVertexBuffer(vertsCount, VertexLayouts::primitiveVertexLayout))
		{
			bgfx::TransientVertexBuffer vb;
			bgfx::allocTransientVertexBuffer(&vb, vertsCount, VertexLayouts::primitiveVertexLayout);
			PosTexCoord0Vertex* vertex = (PosTexCoord0Vertex*)vb.data;

			vertex[0] = { 0, 0, 0, 0, 0 };

			bgfx::setVertexBuffer(0, &vb);

			bgfx::setTransform(glm::value_ptr(trans), 1);
			bgfx::setState(state | BGFX_STATE_POINT_SIZE(size));

			glm::vec4 transparent = glm::vec4(0, 0, 0, 0);
			bgfx::setUniform(Renderer::getSingleton()->u_color, color.ptr(), 1);
			bgfx::setTexture(0, Renderer::getSingleton()->u_albedoMap, Texture::getNullTexture()->getHandle());
			bgfx::setUniform(Renderer::getSingleton()->u_color, color.ptr(), 1);

			bgfx::submit(viewId, program);
		}
	}

	void Primitives::line(glm::mat4x4 trans, glm::vec3 p1, glm::vec3 p2, Color color, int viewId, uint64_t state, bgfx::ProgramHandle program, Camera* camera)
	{
		int vertsCount = 2;

		if (vertsCount == bgfx::getAvailTransientVertexBuffer(vertsCount, VertexLayouts::primitiveVertexLayout))
		{
			bgfx::TransientVertexBuffer vb;
			bgfx::allocTransientVertexBuffer(&vb, vertsCount, VertexLayouts::primitiveVertexLayout);
			PosTexCoord0Vertex* vertex = (PosTexCoord0Vertex*)vb.data;

			vertex[0] = { p1.x, p1.y, p1.z, 0, 0 };
			vertex[1] = { p2.x, p2.y, p2.z, 0, 0 };

			bgfx::setVertexBuffer(0, &vb);

			bgfx::setTransform(glm::value_ptr(trans), 1);
			bgfx::setState(state | BGFX_STATE_PT_LINES/* | BGFX_STATE_LINEAA*/);

			glm::vec4 transparent = glm::vec4(0, 0, 0, 0);
			bgfx::setUniform(Renderer::getSingleton()->u_color, color.ptr(), 1);
			bgfx::setTexture(0, Renderer::getSingleton()->u_albedoMap, Texture::getNullTexture()->getHandle());
			bgfx::setUniform(Renderer::getSingleton()->u_color, color.ptr(), 1);

			bgfx::submit(viewId, program);
		}
	}

	void Primitives::plane(glm::mat4x4 trans, float width, float height, Color color, int viewId, uint64_t state, bgfx::ProgramHandle program, Camera* camera)
	{
		if (4 == bgfx::getAvailTransientVertexBuffer(4, VertexLayouts::primitiveVertexLayout))
		{
			bgfx::TransientVertexBuffer vb;
			bgfx::TransientIndexBuffer ib;
			bgfx::allocTransientVertexBuffer(&vb, 4, VertexLayouts::primitiveVertexLayout);
			bgfx::allocTransientIndexBuffer(&ib, 6);
			PosTexCoord0Vertex* vertex = (PosTexCoord0Vertex*)vb.data;
			uint16_t* index = (uint16_t*)ib.data;

			vertex[0] = { -1.0f,  1.0f,  0.0f, 0, 0 };
			vertex[1] = { 1.0f,  1.0f,  0.0f, 0x7fff, 0 };
			vertex[2] = { -1.0f, -1.0f,  0.0f, 0, 0x7fff };
			vertex[3] = { 1.0f, -1.0f, 0.0f, 0x7fff, 0x7fff };

			index[0] = 0;
			index[1] = 2;
			index[2] = 1;
			index[3] = 1;
			index[4] = 2;
			index[5] = 3;

			bgfx::setVertexBuffer(0, &vb);
			bgfx::setIndexBuffer(&ib);

			bgfx::setTransform(glm::value_ptr(trans), 1);
			bgfx::setState(state);

			bgfx::setTexture(0, Renderer::getSingleton()->u_albedoMap, Texture::getNullTexture()->getHandle());
			bgfx::setUniform(Renderer::getSingleton()->u_color, color.ptr(), 1);

			bgfx::submit(viewId, program);
		}
	}

	void Primitives::grid(glm::mat4x4 trans, int cellCount, float cellSize, Color color, int viewId, uint64_t state, bgfx::ProgramHandle program, Camera* camera)
	{
		int vertsCount = (cellCount * 2) * 2;

		if (vertsCount == bgfx::getAvailTransientVertexBuffer(vertsCount, VertexLayouts::primitiveVertexLayout))
		{
			bgfx::TransientVertexBuffer vb;
			bgfx::allocTransientVertexBuffer(&vb, vertsCount, VertexLayouts::primitiveVertexLayout);
			PosTexCoord0Vertex* vertex = (PosTexCoord0Vertex*)vb.data;

			glm::vec3 camPos = camera->getTransform()->getPosition();
			int startX = camPos.x / cellSize;
			int startY = camPos.z / cellSize;

			int halfSize = (cellCount * cellSize) / 2.0f;

			int idx = 0;
			for (int j = startX; j < startX + cellCount; ++j, idx += 2)
			{
				glm::vec2 p1 = glm::vec2((j * cellSize), (startY * cellSize)) - glm::vec2(halfSize);
				glm::vec2 p2 = glm::vec2((j * cellSize), (startY * cellSize + cellCount * cellSize)) - glm::vec2(halfSize) - glm::vec2(0.0f, cellSize);

				vertex[idx]     = { p1.x, 0, p1.y, 0, 0 };
				vertex[idx + 1] = { p2.x, 0, p2.y, 0, 0 };
			}

			for (int i = startY; i < startY + cellCount; ++i, idx += 2)
			{
				glm::vec2 p1 = glm::vec2((startX * cellSize), (i * cellSize)) - glm::vec2(halfSize);
				glm::vec2 p2 = glm::vec2((startX * cellSize + cellCount * cellSize), (i * cellSize)) - glm::vec2(halfSize) - glm::vec2(cellSize, 0.0f);

				vertex[idx] = { p1.x, 0, p1.y, 0, 0 };
				vertex[idx + 1] = { p2.x, 0, p2.y, 0, 0 };
			}

			bgfx::setVertexBuffer(0, &vb);

			bgfx::setTransform(glm::value_ptr(trans), 1);
			bgfx::setState(state | BGFX_STATE_PT_LINES/* | BGFX_STATE_LINEAA*/);

			glm::vec4 transparent = glm::vec4(0, 0, 0, 0);
			bgfx::setUniform(Renderer::getSingleton()->u_color, color.ptr(), 1);
			bgfx::setTexture(0, Renderer::getSingleton()->u_albedoMap, Texture::getNullTexture()->getHandle());
			bgfx::setUniform(Renderer::getSingleton()->u_color, color.ptr(), 1);

			bgfx::submit(viewId, program);
		}
	}
}