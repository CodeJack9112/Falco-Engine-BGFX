#pragma once

#include "Component.h"

#include "../glm/vec3.hpp"
#include "../glm/gtc/quaternion.hpp"

#include <string>

class btConeTwistConstraint;

namespace GX
{
	class ConeTwistJoint : public Component
	{
	private:
		std::string connectedObjectGuid = "[None]";
		glm::vec3 anchor = glm::vec3(0, 0, 0);
		glm::vec3 connectedAnchor = glm::vec3(0, 0, 0);
		glm::vec3 limits = glm::vec3(3.14f, 3.14f, 3.14f);
		bool linkedBodiesCollision = true;

		btConeTwistConstraint* coneTwistConstraint = nullptr;

		void removeConstraint();

	public:
		ConeTwistJoint();
		virtual ~ConeTwistJoint();

		static std::string COMPONENT_TYPE;
		virtual std::string getComponentType();
		virtual void onSceneLoaded();
		virtual void onStateChanged();
		virtual void onRebindObject(std::string oldObj, std::string newObj);
		virtual Component* onClone();
		virtual void onRefresh();

		std::string getConnectedObjectGuid() { return connectedObjectGuid; }
		void setConnectedObjectGuid(std::string name);

		glm::vec3 getAnchor() { return anchor; }
		void setAnchor(glm::vec3 value) { anchor = value; }

		glm::vec3 getConnectedAnchor() { return connectedAnchor; }
		void setConnectedAnchor(glm::vec3 value) { connectedAnchor = value; }

		glm::vec3 getLimits() { return limits; }
		void setLimits(glm::vec3 value) { limits = value; }

		bool getLinkedBodiesCollision() { return linkedBodiesCollision; }
		void setLinkedBodiesCollision(bool value) { linkedBodiesCollision = value; }

		void autoConfigureAnchors();
	};
}