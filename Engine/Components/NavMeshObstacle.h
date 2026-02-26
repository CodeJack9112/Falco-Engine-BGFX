#pragma once

#include <string>
#include "Component.h"

#include "../glm/vec3.hpp"
#include "../glm/gtc/quaternion.hpp"

namespace GX
{
	class NavMeshObstacle : public Component
	{
	public:
		NavMeshObstacle();
		virtual ~NavMeshObstacle();

		static std::string COMPONENT_TYPE;
		virtual std::string getComponentType();

		virtual void onAttach();
		virtual void onDetach();

		glm::vec3 getSize() { return size; }
		void setSize(glm::vec3 value) { size = value; }

		glm::vec3 getOffset() { return offset; }
		void setOffset(glm::vec3 value) { offset = value; }

		virtual void onSceneLoaded();
		virtual void onStateChanged();
		virtual Component* onClone();

		void update();

	private:
		glm::vec3 size = glm::vec3(1.0f, 1.0f, 1.0f);
		glm::vec3 offset = glm::vec3(0);

		unsigned int obstacleRef = 0;

		glm::vec3 oldPos = glm::vec3(0);
		glm::highp_quat oldRot = glm::identity<glm::highp_quat>();
		glm::vec3 oldScale = glm::vec3(1, 1, 1);
		glm::vec3 oldSize = glm::vec3(1, 1, 1);
		glm::vec3 oldOffset = glm::vec3(1, 1, 1);

		void addObstacle();
		void removeObstacle();
		void updateObstacle();
	};
}