#include "API_Physics.h"

#include <algorithm>

#include "../Core/APIManager.h"
#include "../Core/PhysicsManager.h"
#include "../Components/RigidBody.h"
#include "../Core/LayerMask.h"
#include "../Core/GameObject.h"
#include "../Core/Time.h"

#include <BulletCollision/CollisionDispatch/btGhostObject.h>

namespace GX
{
	struct CustomCollisionCallback : public btCollisionWorld::ConvexResultCallback
	{
		std::vector<const btCollisionObject*> mHits;

		btScalar addSingleResult(btDynamicsWorld::LocalConvexResult& convexResult, bool normalInWorldSpace)
		{
			if (std::find(mHits.begin(), mHits.end(), convexResult.m_hitCollisionObject) == mHits.end())
				mHits.push_back(convexResult.m_hitCollisionObject);

			return convexResult.m_hitFraction;
		}
	};

	void API_Physics::raycast(API::Vector3* ref_from, API::Vector3* ref_to, unsigned long layer, API::RaycastHit* out_hit)
	{
		btVector3 btFrom(ref_from->x, ref_from->y, ref_from->z);
		btVector3 btTo(ref_to->x, ref_to->y, ref_to->z);
		btCollisionWorld::AllHitsRayResultCallback res(btFrom, btTo);

		LayerMask layerMask;
		layerMask.fromULong(layer);

		btDynamicsWorld* world = PhysicsManager::getSingleton()->getWorld();
		world->rayTest(btFrom, btTo, res);

		API::Vector3 point;
		API::Vector3 normal;
		MonoObject* _body = nullptr;

		point.x = 0;
		point.y = 0;
		point.z = 0;

		normal.x = 0;
		normal.y = 0;
		normal.z = 0;

		float closestDist = FLT_MAX;

		for (int i = 0; i < res.m_collisionObjects.size(); ++i)
		{
			btRigidBody* body = const_cast<btRigidBody*>(btRigidBody::upcast(res.m_collisionObjects[i]));

			float d = res.m_hitPointWorld[i].distance(btFrom);
			if (d < closestDist)
			{
				std::vector<RigidBody*>& bodies = PhysicsManager::getSingleton()->getBodies();
				for (auto& it : bodies)
				{
					GameObject* obj = it->getGameObject();

					if (it->getNativeBody() == body)
					{
						if (layerMask.getLayer(obj->getLayer()))
						{
							_body = it->getManagedObject();

							point.x = res.m_hitPointWorld[i].getX();
							point.y = res.m_hitPointWorld[i].getY();
							point.z = res.m_hitPointWorld[i].getZ();

							normal.x = res.m_hitNormalWorld[i].getX();
							normal.y = res.m_hitNormalWorld[i].getY();
							normal.z = res.m_hitNormalWorld[i].getZ();

							closestDist = d;
						}

						break;
					}
				}
			}
		}

		out_hit->rigidBody = _body;
		out_hit->hitPoint = point;
		out_hit->worldNormal = normal;
		out_hit->hasHit = res.hasHit();
	}

	MonoArray* API_Physics::overlapSphere(API::Vector3* center, float radius, unsigned long layer)
	{
		btVector3 btFrom = btVector3(center->x, center->y, center->z);
		btVector3 btTo = btVector3(center->x, center->y - 0.0001f, center->z);
		btTransform transform1;
		btTransform transform2;
		transform1.setBasis(btMatrix3x3::getIdentity());
		transform2.setBasis(btMatrix3x3::getIdentity());
		transform1.setOrigin(btFrom);
		transform2.setOrigin(btTo);

		LayerMask layerMask;
		layerMask.fromULong(layer);

		btSphereShape sphere = btSphereShape(radius);

		CustomCollisionCallback res = CustomCollisionCallback();

		btDynamicsWorld* world = PhysicsManager::getSingleton()->getWorld();

		world->convexSweepTest(&sphere, transform1, transform2, res);

		std::vector<RigidBody*>& bodies = PhysicsManager::getSingleton()->getBodies();
		std::vector<MonoObject*> _bodies;

		for (auto _it = res.mHits.begin(); _it != res.mHits.end(); ++_it)
		{
			btRigidBody* body = const_cast<btRigidBody*>(btRigidBody::upcast(*_it));
			bool hasBody = false;

			for (auto& it : bodies)
			{
				GameObject* obj = it->getGameObject();

				if (it->getNativeBody() == body)
				{
					if (layerMask.getLayer(obj->getLayer()))
						_bodies.push_back(it->getManagedObject());

					hasBody = true;

					break;
				}
			}

			if (!hasBody)
				_bodies.push_back(nullptr); //CSG or other static geometry
		}

		MonoArray* arr = mono_array_new(APIManager::getSingleton()->getDomain(), APIManager::getSingleton()->collider_class, _bodies.size());

		for (int j = 0; j < _bodies.size(); ++j)
			mono_array_setref(arr, j, _bodies[j]);

		_bodies.clear();

		return arr;
	}
}