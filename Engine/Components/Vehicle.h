#pragma once

#include "Component.h"
#include <string>
#include <vector>

#include "../Bullet/include/btBulletCollisionCommon.h"
#include "../Bullet/include/btBulletDynamicsCommon.h"

#include "../glm/vec3.hpp"

namespace GX
{
	class Vehicle : public Component
	{
	public:
		struct WheelInfo
		{
		public:
			std::string m_connectedObjectGuid = "[None]";
			float m_radius = 0.5f;
			float m_width = 0.4f;
			float m_suspensionStiffness = 20.0f;
			float m_suspensionDamping = 2.3f;
			float m_suspensionCompression = 4.4f;
			float m_suspensionRestLength = 0.6f;
			float m_friction = 1000.0f;
			float m_rollInfluence = 0.1f;
			glm::vec3 m_direction = glm::vec3(0, -1, 0);
			glm::vec3 m_axle = glm::vec3(-1, 0, 0);
			glm::vec3 m_connectionPoint = glm::vec3(0, 0, 0);
			bool m_isFrontWheel = false;

			Transform* m_connectedObjectRef = nullptr;
		};

	private:
		const int maxProxies = 32766;
		const int maxOverlap = 65535;

		btRaycastVehicle::btVehicleTuning m_tuning;
		btRaycastVehicle* m_vehicle = nullptr;

		glm::vec3 vehicleAxis = glm::vec3(0, 1, 2);
		bool invertForward = false;

		std::vector<Vehicle::WheelInfo> wheels;

	public:
		Vehicle();
		virtual ~Vehicle();

		static std::string COMPONENT_TYPE;
		virtual std::string getComponentType();
		virtual Component* onClone();
		virtual void onSceneLoaded();
		virtual void onStateChanged();
		virtual void onRebindObject(std::string oldObj, std::string newObj);
		virtual void onRefresh();
		virtual void onAttach();
		virtual void onDetach();

		void autoConfigureAnchors(int wheelIndex);
		std::vector<WheelInfo>& getWheels() { return wheels; }

		void update();
		btRaycastVehicle* getVehicle() { return m_vehicle; }

		glm::vec3 getAxis() { return vehicleAxis; }
		void setAxis(glm::vec3 value) { vehicleAxis = value; }

		bool getInvertForward() { return invertForward; }
		void setInvertForward(bool value) { invertForward = value; }
	};
}