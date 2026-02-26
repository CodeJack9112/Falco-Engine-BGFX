#include "Vehicle.h"

#include "RigidBody.h"
#include "../Math/Mathf.h"
#include "../Core/PhysicsManager.h"
#include "../Core/APIManager.h"
#include "../Core/GameObject.h"
#include "../Components/Transform.h"
#include "../Core/Engine.h"

namespace GX
{
	std::string Vehicle::COMPONENT_TYPE = "Vehicle";

	Vehicle::Vehicle() : Component(APIManager::getSingleton()->vehicle_class)
	{
		
	}

	Vehicle::~Vehicle()
	{
		if (m_vehicle != nullptr)
		{
			PhysicsManager::getSingleton()->getWorld()->removeAction(m_vehicle);
			delete m_vehicle;
		}
	}

	std::string Vehicle::getComponentType()
	{
		return COMPONENT_TYPE;
	}

	Component* Vehicle::onClone()
	{
		Vehicle* newComponent = new Vehicle();
		newComponent->enabled = enabled;
		newComponent->vehicleAxis = vehicleAxis;
		newComponent->invertForward = invertForward;
		newComponent->wheels = wheels;

		return newComponent;
	}

	void Vehicle::autoConfigureAnchors(int wheelIndex)
	{
		WheelInfo & inf = wheels.at(wheelIndex);

		GameObject* connectedNode = Engine::getSingleton()->getGameObject(inf.m_connectedObjectGuid);
		if (connectedNode != nullptr)
		{
			inf.m_connectionPoint = Mathf::inverseTransformPoint(getGameObject()->getTransform(), connectedNode->getTransform()->getPosition());
		}
	}

	btVector3 bulletVec3(glm::vec3 vec3)
	{
		return btVector3(vec3.x, vec3.y, vec3.z);
	}

	glm::vec3 vec3Bullet(btVector3 vec3)
	{
		return glm::vec3(vec3.getX(), vec3.getY(), vec3.getZ());
	}

	glm::highp_quat quatBullet(btQuaternion quat)
	{
		return glm::highp_quat(quat.getW(), quat.getX(), quat.getY(), quat.getZ());
	}

	btQuaternion bulletQuat(glm::highp_quat quat)
	{
		return btQuaternion(quat.x, quat.y, quat.z, quat.w);
	}

	void Vehicle::onSceneLoaded()
	{
		if (gameObject == nullptr)
			return;

		if (!getEnabled() || !gameObject->getActive())
			return;

		if (Engine::getSingleton()->getIsRuntimeMode())
		{
			RigidBody* body = (RigidBody*)getGameObject()->getComponent(RigidBody::COMPONENT_TYPE);

			if (body != nullptr)
			{
				btRigidBody* body1 = body->getNativeBody();

				if (body1 != nullptr)
				{
					body1->setActivationState(DISABLE_DEACTIVATION);

					PhysicsManager* mgr = PhysicsManager::getSingleton();

					m_vehicle = new btRaycastVehicle(m_tuning, body1, mgr->m_vehicleRayCaster);

					//choose coordinate system
					m_vehicle->setCoordinateSystem(vehicleAxis.x, vehicleAxis.y, vehicleAxis.z);

					Transform* transform = getGameObject()->getTransform();
					float scale = transform->getScale().y;
					glm::highp_quat rotation = transform->getRotation();

					for (auto it = wheels.begin(); it != wheels.end(); ++it)
					{
						WheelInfo & wheelInfo = *it;
						
						glm::vec3 offset = vec3Bullet(body1->getCenterOfMassTransform().getOrigin()) - transform->getPosition();

						btWheelInfo& wheel = m_vehicle->addWheel(bulletVec3(wheelInfo.m_connectionPoint * transform->getScale() - offset),
							bulletVec3(glm::inverse(rotation) * wheelInfo.m_direction),
							bulletVec3(glm::inverse(rotation) * wheelInfo.m_axle),
							wheelInfo.m_suspensionRestLength,
							wheelInfo.m_radius * scale,
							m_tuning,
							wheelInfo.m_isFrontWheel);

						wheel.m_suspensionStiffness = wheelInfo.m_suspensionStiffness;
						wheel.m_wheelsDampingRelaxation = wheelInfo.m_suspensionDamping;
						wheel.m_wheelsDampingCompression = wheelInfo.m_suspensionCompression;
						wheel.m_frictionSlip = wheelInfo.m_friction;
						wheel.m_rollInfluence = wheelInfo.m_rollInfluence;
						wheel.m_maxSuspensionForce = 100000;
						//wheel.m_maxSuspensionTravelCm = 20.0f;

						if (!wheelInfo.m_connectedObjectGuid.empty())
						{
							GameObject* obj = Engine::getSingleton()->getGameObject(wheelInfo.m_connectedObjectGuid);
							if (obj != nullptr)
								wheelInfo.m_connectedObjectRef = obj->getTransform();
						}
					}
					
					PhysicsManager::getSingleton()->getWorld()->addAction(m_vehicle);
					m_vehicle->resetSuspension();
				}
			}
		}
	}

	void Vehicle::update()
	{
		if (m_vehicle == nullptr)
			return;

		for (int i = 0; i < m_vehicle->getNumWheels(); i++)
		{
			btWheelInfo& wheel = m_vehicle->getWheelInfo(i);
			
			wheel.m_raycastInfo.m_isInContact = false;

			btTransform chassisTrans = m_vehicle->getRigidBody()->getCenterOfMassTransform();
			if ((m_vehicle->getRigidBody()->getMotionState()))
			{
				m_vehicle->getRigidBody()->getMotionState()->getWorldTransform(chassisTrans);
			}

			wheel.m_raycastInfo.m_hardPointWS = chassisTrans(wheel.m_chassisConnectionPointCS);
			wheel.m_raycastInfo.m_wheelDirectionWS = chassisTrans.getBasis() * wheel.m_wheelDirectionCS;
			wheel.m_raycastInfo.m_wheelAxleWS = chassisTrans.getBasis() * wheel.m_wheelAxleCS;

			btVector3 up = -wheel.m_raycastInfo.m_wheelDirectionWS;
			const btVector3& right = wheel.m_raycastInfo.m_wheelAxleWS;
			btVector3 fwd = up.cross(right);
			fwd = fwd.normalize();

			btScalar steering = wheel.m_steering;

			btQuaternion steeringOrn(up, steering);
			btMatrix3x3 steeringMat(steeringOrn);

			btQuaternion rotatingOrn(right, invertForward ? wheel.m_rotation : -wheel.m_rotation);
			btMatrix3x3 rotatingMat(rotatingOrn);

			btMatrix3x3 basis2;
			basis2[0][0] = -right[0];
			basis2[1][0] = -right[1];
			basis2[2][0] = -right[2];

			basis2[0][1] = up[0];
			basis2[1][1] = up[1];
			basis2[2][1] = up[2];

			basis2[0][2] = fwd[0];
			basis2[1][2] = fwd[1];
			basis2[2][2] = fwd[2];

			wheel.m_worldTransform.setBasis(steeringMat * rotatingMat * basis2);
			wheel.m_worldTransform.setOrigin(
				wheel.m_raycastInfo.m_hardPointWS + wheel.m_raycastInfo.m_wheelDirectionWS * wheel.m_raycastInfo.m_suspensionLength);

			/////////////

			btWheelInfo& winf = m_vehicle->getWheelInfo(i);

			glm::vec3 position = vec3Bullet(winf.m_worldTransform.getOrigin());
			glm::highp_quat rotation = quatBullet(winf.m_worldTransform.getRotation());

			WheelInfo & inf = wheels.at(i);
			if (inf.m_connectedObjectRef != nullptr)
			{
				inf.m_connectedObjectRef->setPosition(position);
				inf.m_connectedObjectRef->setRotation(rotation);
			}
		}
	}

	void Vehicle::onStateChanged()
	{
		if (!Engine::getSingleton()->getIsRuntimeMode())
			return;

		if (gameObject == nullptr)
			return;

		bool active = getEnabled() && gameObject->getActive();

		RigidBody* body = (RigidBody*)gameObject->getComponent(RigidBody::COMPONENT_TYPE);

		if (active)
		{
			if (body != nullptr)
			{
				if (body->isInitialized())
				{
					auto& vehicles = PhysicsManager::getSingleton()->getVehicles();

					if (std::find(vehicles.begin(), vehicles.end(), this) == vehicles.end())
					{
						onSceneLoaded();
						PhysicsManager::getSingleton()->addVehicle(this);
					}
				}
			}
		}
		else
		{
			if (m_vehicle != nullptr)
			{
				PhysicsManager::getSingleton()->getWorld()->removeAction(m_vehicle);
				delete m_vehicle;
			}

			PhysicsManager::getSingleton()->removeVehicle(this);
		}
	}

	void Vehicle::onRebindObject(std::string oldObj, std::string newObj)
	{
		for (auto it = wheels.begin(); it != wheels.end(); ++it)
		{
			WheelInfo& inf = *it;

			if (inf.m_connectedObjectGuid == oldObj)
			{
				inf.m_connectedObjectGuid = newObj;
				inf.m_connectedObjectRef = Engine::getSingleton()->getGameObject(newObj)->getTransform();
			}
		}
	}

	void Vehicle::onRefresh()
	{
		setEnabled(false);
		setEnabled(true);
	}

	void Vehicle::onAttach()
	{
		Component::onAttach();

		if (Engine::getSingleton()->getIsRuntimeMode())
		{
			if (enabled && gameObject->getActive())
				PhysicsManager::getSingleton()->addVehicle(this);
		}
	}

	void Vehicle::onDetach()
	{
		Component::onDetach();

		if (Engine::getSingleton()->getIsRuntimeMode())
			PhysicsManager::getSingleton()->removeVehicle(this);
	}
}