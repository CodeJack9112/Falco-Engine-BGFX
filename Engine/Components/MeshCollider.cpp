#include "MeshCollider.h"
#include "RigidBody.h"

#include "../Bullet/include/btBulletCollisionCommon.h"
#include "../Bullet/include/btBulletDynamicsCommon.h"

#include "../Core/GameObject.h"
#include "../Components/MeshRenderer.h"
#include "../Assets/Mesh.h"
#include "../Core/APIManager.h"

namespace GX
{
	std::string MeshCollider::COMPONENT_TYPE = "MeshCollider";

	MeshCollider::MeshCollider() : Collider(APIManager::getSingleton()->meshcollider_class)
	{
		
	}

	MeshCollider::~MeshCollider()
	{
		if (colMesh != nullptr)
			delete colMesh;

		colMesh = nullptr;
	}

	std::string MeshCollider::getComponentType()
	{
		return COMPONENT_TYPE;
	}

	Component* MeshCollider::onClone()
	{
		MeshCollider* newComponent = new MeshCollider();
		newComponent->enabled = enabled;
		newComponent->offset = offset;
		newComponent->rotation = rotation;
		newComponent->isTrigger = isTrigger;
		newComponent->convex = convex;

		return newComponent;
	}

	void MeshCollider::rebuild()
	{
		if (colMesh != nullptr)
		{
			delete colMesh;
			colMesh = nullptr;
		}

		if (collisionShape != nullptr)
		{
			delete collisionShape;
			collisionShape = nullptr;
		}

		if (getGameObject() == nullptr)
			return;

		MeshRenderer* rend = (MeshRenderer*)getGameObject()->getComponent(MeshRenderer::COMPONENT_TYPE);
		if (rend != nullptr && rend->getMesh() != nullptr)
		{
			Mesh* mesh = rend->getMesh();

			if (convex)
				collisionShape = new btConvexHullShape();
			else
				colMesh = new btTriangleMesh();
			
			for (int sm = 0; sm < mesh->getSubMeshCount(); ++sm)
			{
				SubMesh* subMesh = mesh->getSubMesh(sm);
				std::vector<VertexBuffer> & vbuf = subMesh->getVertexBuffer();
				std::vector<uint32_t> & ibuf = subMesh->getIndexBuffer();

				if (convex)
				{
					for (auto i = vbuf.begin(); i != vbuf.end(); ++i)
					{
						glm::vec3 pt = i->position;

						((btConvexHullShape*)collisionShape)->addPoint(btVector3(btScalar(pt.x), btScalar(pt.y), btScalar(pt.z)));
					}
				}
				else
				{
					for (int i = 0; i < ibuf.size(); i += 3)
					{
						glm::vec3 pt1 = vbuf[ibuf[i]].position;
						glm::vec3 pt2 = vbuf[ibuf[i + 1]].position;
						glm::vec3 pt3 = vbuf[ibuf[i + 2]].position;

						colMesh->addTriangle(
							btVector3(btScalar(pt1.x), btScalar(pt1.y), btScalar(pt1.z)),
							btVector3(btScalar(pt2.x), btScalar(pt2.y), btScalar(pt2.z)),
							btVector3(btScalar(pt3.x), btScalar(pt3.y), btScalar(pt3.z)));
					}
				}
			}

			if (convex)
				((btConvexHullShape*)collisionShape)->recalcLocalAabb();
			else
			{
				if (colMesh->getNumTriangles() > 0)
				{
					collisionShape = new btBvhTriangleMeshShape(colMesh, true);
					((btBvhTriangleMeshShape*)collisionShape)->recalcLocalAabb();
				}
			}

			setCollisionShape(collisionShape);
		}
	}

	void MeshCollider::setConvex(bool c)
	{
		convex = c;

		rebuild();
	}

	void MeshCollider::onAttach()
	{
		Component::onAttach();

		rebuild();
	}

	void MeshCollider::onSceneLoaded()
	{
		rebuild();
	}
}