#pragma once

#include <string>
#include "Component.h"

#include "../glm/vec3.hpp"
#include "../glm/gtc/quaternion.hpp"

#define MAX_PATHSLOT		1 // how many paths we can store
#define MAX_PATHPOLY		2048 // max number of polygons in a path
#define MAX_PATHVERT		2048 // max verts in a path 

typedef unsigned int dtPolyRef;

namespace GX
{
	//class DynamicLines;

	struct PathData
	{
		float PosX[MAX_PATHVERT];
		float PosY[MAX_PATHVERT];
		float PosZ[MAX_PATHVERT];
		int VertCount = 0;
		bool valid = true;
	};

	class NavMeshAgent : public Component
	{
	private:
		float radius = 0.6f;
		float height = 2.0f;
		float speed = 3.5f;
		float acceleration = 8.0f;
		float rotationSpeed = 1.0f;

		glm::vec3 targetPosition = glm::vec3(0);

		PathData findPath(glm::vec3 StartPos, glm::vec3 EndPos);

		//DynamicLines * debugPath = nullptr;
		PathData currentPath;

	public:
		NavMeshAgent();
		virtual ~NavMeshAgent();

		static std::string COMPONENT_TYPE;
		virtual std::string getComponentType();
		virtual Component* onClone();

		void update();

		void setTargetPosition(glm::vec3 pos);
		glm::vec3 getTargetPosition() { return targetPosition; }

		float getRadius() { return radius; }
		void setRadius(float r);

		float getHeight() { return height; }
		void setHeight(float h);

		float getSpeed() { return speed; }
		void setSpeed(float s);

		float getAcceleration() { return acceleration; }
		void setAcceleration(float a);

		float getRotationSpeed() { return rotationSpeed; }
		void setRotationSpeed(float s);
	};
}