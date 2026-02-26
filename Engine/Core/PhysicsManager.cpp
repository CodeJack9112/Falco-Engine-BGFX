#include "PhysicsManager.h"

#include <iostream>
#include <algorithm>

#include "Components/RigidBody.h"
#include "Components/Vehicle.h"

#include "Engine.h"
#include "Core/Debug.h"
#include "Core/APIManager.h"
#include "Core/Time.h"
#include "Core/GameObject.h"
#include "Renderer/Renderer.h"

//#include <BulletCollision/CollisionDispatch/btGhostObject.h>

namespace GX
{
	PhysicsManager PhysicsManager::singleton;

	PhysicsManager::PhysicsManager()
	{
	}

	PhysicsManager::~PhysicsManager()
	{
	}

	void PhysicsManager::init()
	{
		collisionConfiguration = new btDefaultCollisionConfiguration();
		dispatcher = new btCollisionDispatcher(collisionConfiguration);
		overlappingPairCache = new btDbvtBroadphase();
		solver = new btSequentialImpulseConstraintSolver();

		dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, overlappingPairCache, solver, collisionConfiguration);
		dynamicsWorld->setGravity(btVector3(0.0f, -9.81f, 0.0f));
		dynamicsWorld->setLatencyMotionStateInterpolation(true);

		m_vehicleRayCaster = new btDefaultVehicleRaycaster(dynamicsWorld);

		//overlappingPairCache->getOverlappingPairCache()->setInternalGhostPairCallback(new btGhostPairCallback());

		if (!Engine::getSingleton()->getAssetsPath().empty())
			Debug::log("Physics initialized", Debug::DbgColorGreen);
	}

	void PhysicsManager::free()
	{
		for (std::vector<RigidBody*>::iterator it = bodies.begin(); it != bodies.end(); ++it)
		{
			if ((*it)->getNativeBody() != nullptr)
				dynamicsWorld->removeRigidBody((*it)->getNativeBody());
		}

		bodies.clear();

		delete dynamicsWorld;
		delete solver;
		delete overlappingPairCache;
		delete dispatcher;
		delete collisionConfiguration;
	}

	void PhysicsManager::update()
	{
		if (!Engine::getSingleton()->getIsRuntimeMode())
			return;

		if (dynamicsWorld == nullptr)
			return;

		//Update all rigidbodies
		for (std::vector<RigidBody*>::iterator it = bodies.begin(); it != bodies.end(); ++it)
		{
			(*it)->update();
		}

		//Update all vehicles
		for (std::vector<Vehicle*>::iterator it = vehicles.begin(); it != vehicles.end(); ++it)
		{
			(*it)->update();
		}

		//Physics step
		float fixedDt = 1.0f / 120.0f;
		float dt = Time::getDeltaTime() * Time::getTimeScale();

		dynamicsWorld->stepSimulation(dt, 10, fixedDt);

		checkCollisions();
	}

	void PhysicsManager::checkCollisions()
	{
		std::map<btCollisionObject*, CollisionInfo> newContacts;

		/* Browse all collision pairs */
		int numManifolds = dynamicsWorld->getDispatcher()->getNumManifolds();
		for (int i = 0; i < numManifolds; i++)
		{
			btPersistentManifold* contactManifold = dynamicsWorld->getDispatcher()->getManifoldByIndexInternal(i);
			btCollisionObject* obA = const_cast<btCollisionObject*>(contactManifold->getBody0());
			btCollisionObject* obB = const_cast<btCollisionObject*>(contactManifold->getBody1());

			/* Check all contacts points */
			int numContacts = contactManifold->getNumContacts();
			for (int j = 0; j < numContacts; j++)
			{
				btManifoldPoint& pt = contactManifold->getContactPoint(j);
				if (pt.getDistance() < 0.f)
				{
					const btVector3& ptA = pt.getPositionWorldOnA();
					const btVector3& ptB = pt.getPositionWorldOnB();
					const btVector3& normalOnB = pt.m_normalWorldOnB;

					if (newContacts.find(obB) == newContacts.end())
					{
						newContacts[obB] = CollisionInfo(obA, ptA, ptB, normalOnB);
					}
				}
			}
		}

		/* Check for added contacts ... */
		if (!newContacts.empty())
		{
			for (auto it = newContacts.begin(); it != newContacts.end(); it++)
			{
				if (m_contacts.find((*it).first) == m_contacts.end())
				{
					//std::cout << "Collision detected" << std::endl;
					
					//Signal
					for (RigidBody * body : bodies)
					{
						if (body->getNativeBody() == it->second.obA)
						{
							GameObject* thisNode = body->getGameObject();
							RigidBody* otherBody = nullptr;

							for (RigidBody * body2 : bodies)
							{
								if (body2->getNativeBody() == it->first)
								{
									otherBody = body2;
									break;
								}
							}

							if (otherBody != nullptr)
							{
								btVector3 ptx = it->second.ptB;
								btVector3 nmx = it->second.normalOnB;

								float px = ptx.x();
								float py = ptx.y();
								float pz = ptx.z();

								float nx = nmx.x();
								float ny = nmx.y();
								float nz = nmx.z();

								//Contact point
								MonoObject* pt = mono_object_new(APIManager::getSingleton()->getDomain(), APIManager::getSingleton()->vector3_class);
								mono_field_set_value(pt, APIManager::getSingleton()->vector3_x, &px);
								mono_field_set_value(pt, APIManager::getSingleton()->vector3_y, &py);
								mono_field_set_value(pt, APIManager::getSingleton()->vector3_z, &pz);

								void* _point = mono_object_unbox(pt);

								//Contact normal
								MonoObject* nm = mono_object_new(APIManager::getSingleton()->getDomain(), APIManager::getSingleton()->vector3_class);
								mono_field_set_value(nm, APIManager::getSingleton()->vector3_x, &nx);
								mono_field_set_value(nm, APIManager::getSingleton()->vector3_y, &ny);
								mono_field_set_value(nm, APIManager::getSingleton()->vector3_z, &nz);

								void* _normal = mono_object_unbox(nm);

								//A
								MonoObject* collisionStructA = mono_object_new(APIManager::getSingleton()->getDomain(), APIManager::getSingleton()->collision_class);
								mono_field_set_value(collisionStructA, APIManager::getSingleton()->collision_other, otherBody->getManagedObject());
								mono_field_set_value(collisionStructA, APIManager::getSingleton()->collision_point, _point);
								mono_field_set_value(collisionStructA, APIManager::getSingleton()->collision_normal, _normal);

								//B
								MonoObject* collisionStructB = mono_object_new(APIManager::getSingleton()->getDomain(), APIManager::getSingleton()->collision_class);
								mono_field_set_value(collisionStructB, APIManager::getSingleton()->collision_other, body->getManagedObject());
								mono_field_set_value(collisionStructB, APIManager::getSingleton()->collision_point, _point);
								mono_field_set_value(collisionStructB, APIManager::getSingleton()->collision_normal, _normal);

								void* _colA = mono_object_unbox(collisionStructA);
								void* _colB = mono_object_unbox(collisionStructB);

								void* argsA[1] = { _colA };
								void* argsB[1] = { _colB };

								GameObject* otherNode = otherBody->getGameObject();

								APIManager::getSingleton()->executeForNode(thisNode, "OnCollisionEnter", "", argsA, "Collision");
								APIManager::getSingleton()->executeForNode(otherNode, "OnCollisionEnter", "", argsB, "Collision");

								if (body->getIsTrigger())
								{
									void* argsC[1] = { otherBody->getManagedObject() };
									APIManager::getSingleton()->executeForNode(thisNode, "OnTriggerEnter", "", argsC, "Rigidbody");
								}

								if (otherBody->getIsTrigger())
								{
									void* argsC[1] = { body->getManagedObject() };
									APIManager::getSingleton()->executeForNode(otherNode, "OnTriggerEnter", "", argsC, "Rigidbody");
								}

								break;
							}
						}
					}
				}
				else
				{
					// Remove to filter no more active contacts
					m_contacts.erase((*it).first);
				}
			}
		}

		/* ... and removed contacts */
		if (!m_contacts.empty())
		{
			for (auto it = m_contacts.begin(); it != m_contacts.end(); it++)
			{
				//std::cout << "End of collision detected" << std::endl;
				
				//Signal
				for (RigidBody * body : bodies)
				{
					if (body->getNativeBody() == it->second.obA)
					{
						GameObject* thisNode = body->getGameObject();
						RigidBody* otherBody = nullptr;

						for (RigidBody * body2 : bodies)
						{
							if (body2->getNativeBody() == it->first)
							{
								otherBody = body2;
								break;
							}
						}

						if (otherBody != nullptr)
						{
							btVector3 ptx = it->second.ptB;
							btVector3 nmx = it->second.normalOnB;

							float px = ptx.x();
							float py = ptx.y();
							float pz = ptx.z();

							float nx = nmx.x();
							float ny = nmx.y();
							float nz = nmx.z();

							//Contact point
							MonoObject* pt = mono_object_new(APIManager::getSingleton()->getDomain(), APIManager::getSingleton()->vector3_class);
							mono_field_set_value(pt, APIManager::getSingleton()->vector3_x, &px);
							mono_field_set_value(pt, APIManager::getSingleton()->vector3_y, &py);
							mono_field_set_value(pt, APIManager::getSingleton()->vector3_z, &pz);

							void* _point = mono_object_unbox(pt);

							//Contact normal
							MonoObject* nm = mono_object_new(APIManager::getSingleton()->getDomain(), APIManager::getSingleton()->vector3_class);
							mono_field_set_value(nm, APIManager::getSingleton()->vector3_x, &nx);
							mono_field_set_value(nm, APIManager::getSingleton()->vector3_y, &ny);
							mono_field_set_value(nm, APIManager::getSingleton()->vector3_z, &nz);

							void* _normal = mono_object_unbox(nm);

							//A
							MonoObject* collisionStructA = mono_object_new(APIManager::getSingleton()->getDomain(), APIManager::getSingleton()->collision_class);
							mono_field_set_value(collisionStructA, APIManager::getSingleton()->collision_other, otherBody->getManagedObject());
							mono_field_set_value(collisionStructA, APIManager::getSingleton()->collision_point, _point);
							mono_field_set_value(collisionStructA, APIManager::getSingleton()->collision_normal, _normal);

							//B
							MonoObject* collisionStructB = mono_object_new(APIManager::getSingleton()->getDomain(), APIManager::getSingleton()->collision_class);
							mono_field_set_value(collisionStructB, APIManager::getSingleton()->collision_other, body->getManagedObject());
							mono_field_set_value(collisionStructB, APIManager::getSingleton()->collision_point, _point);
							mono_field_set_value(collisionStructB, APIManager::getSingleton()->collision_normal, _normal);

							void* _colA = mono_object_unbox(collisionStructA);
							void* _colB = mono_object_unbox(collisionStructB);

							void* argsA[1] = { _colA };
							void* argsB[1] = { _colB };

							GameObject* otherNode = otherBody->getGameObject();

							APIManager::getSingleton()->executeForNode(thisNode, "OnCollisionExit", "", argsA, "Collision");
							APIManager::getSingleton()->executeForNode(otherNode, "OnCollisionExit", "", argsB, "Collision");

							if (body->getIsTrigger())
							{
								void* argsC[1] = { otherBody->getManagedObject() };
								APIManager::getSingleton()->executeForNode(thisNode, "OnTriggerExit", "", argsC, "Rigidbody");
							}

							if (otherBody->getIsTrigger())
							{
								void* argsC[1] = { body->getManagedObject() };
								APIManager::getSingleton()->executeForNode(otherNode, "OnTriggerExit", "", argsC, "Rigidbody");
							}

							break;
						}
					}
				}
			}

			m_contacts.clear();
		}

		m_contacts = newContacts;
		newContacts.clear();
	}

	void PhysicsManager::reset()
	{
		free();
		init();
	}

	void PhysicsManager::addBody(RigidBody * body)
	{
		auto it = std::find(bodies.begin(), bodies.end(), body);
		if (it == bodies.end())
			bodies.push_back(body);
	}

	void PhysicsManager::removeBody(RigidBody * body)
	{
		auto it = std::find(bodies.begin(), bodies.end(), body);
		if (it != bodies.end())
			bodies.erase(it);
	}

	void PhysicsManager::addVehicle(Vehicle* vehicle)
	{
		vehicles.push_back(vehicle);
	}

	void PhysicsManager::removeVehicle(Vehicle* vehicle)
	{
		auto it = std::find(vehicles.begin(), vehicles.end(), vehicle);
		if (it != vehicles.end())
			vehicles.erase(it);
	}

	void PhysicsManager::updateCollisionMatrix()
	{
		if (!Engine::getSingleton()->getIsRuntimeMode())
			return;

		ProjectSettings* settings = Engine::getSingleton()->getSettings();

		for (auto& body1 : bodies)
		{
			GameObject* obj1 = body1->getGameObject();
			btRigidBody* rb1 = body1->getNativeBody();

			for (auto& body2 : bodies)
			{
				GameObject* obj2 = body2->getGameObject();
				btRigidBody* rb2 = body2->getNativeBody();

				int l1 = obj1->getLayer();
				int l2 = obj2->getLayer();

				bool collision = settings->getCollisionMask(l1, l2) || settings->getCollisionMask(l2, l1);

				rb1->setIgnoreCollisionCheck(rb2, !collision);
				rb2->setIgnoreCollisionCheck(rb1, !collision);
			}
		}
	}
}