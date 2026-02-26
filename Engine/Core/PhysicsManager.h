#pragma once

#include <map>
#include <vector>

#include "../Bullet/include/btBulletCollisionCommon.h"
#include "../Bullet/include/btBulletDynamicsCommon.h"

namespace GX
{
	class RigidBody;
	class Vehicle;
	class Transform;

	class PhysicsManager
	{
		friend class RigidBody;
		friend class Vehicle;

	private:
		struct CollisionInfo
		{
		public:
			CollisionInfo() {}
			CollisionInfo(btCollisionObject* obA, const btVector3 ptA, const btVector3 ptB, const btVector3 normalOnB)
			{
				this->obA = obA;
				this->ptA = ptA;
				this->ptB = ptB;
				this->normalOnB = normalOnB;
			}

			btCollisionObject* obA = nullptr;
			btVector3 ptA;
			btVector3 ptB;
			btVector3 normalOnB;
		};

		static PhysicsManager singleton;

		btDiscreteDynamicsWorld* dynamicsWorld = nullptr;
		btDefaultCollisionConfiguration* collisionConfiguration = nullptr;
		btCollisionDispatcher* dispatcher = nullptr;
		btBroadphaseInterface* overlappingPairCache = nullptr;
		btSequentialImpulseConstraintSolver* solver = nullptr;

		btVehicleRaycaster* m_vehicleRayCaster = nullptr;

		std::vector<RigidBody*> bodies;
		std::vector<Vehicle*> vehicles;

		std::map<btCollisionObject*, CollisionInfo> m_contacts;

		void checkCollisions();
		
		void addBody(RigidBody* body);
		void removeBody(RigidBody* body);
		void addVehicle(Vehicle* body);
		void removeVehicle(Vehicle* body);

	public:
		PhysicsManager();
		~PhysicsManager();

		static PhysicsManager* getSingleton() { return &singleton; }

		void init();
		void free();
		void update();
		void reset();

		btDiscreteDynamicsWorld* getWorld() { return dynamicsWorld; }

		std::vector<RigidBody*>& getBodies() { return bodies; }
		std::vector<Vehicle*>& getVehicles() { return vehicles; }

		void updateCollisionMatrix();
	};
}