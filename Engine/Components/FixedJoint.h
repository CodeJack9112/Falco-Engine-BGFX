#pragma once

#include <string>

#include "../glm/vec3.hpp"

#include "Component.h"

class btFixedConstraint;

namespace GX
{
	class FixedJoint : public Component
	{
	private:
		std::string connectedObjectGuid = "[None]";
		glm::vec3 anchor = glm::vec3(0, 0, 0);
		glm::vec3 connectedAnchor = glm::vec3(0, 0, 0);

		bool linkedBodiesCollision = true;

		btFixedConstraint* fixedConstraint = nullptr;

		void removeConstraint();

	public:
		FixedJoint();
		virtual ~FixedJoint();

		static std::string COMPONENT_TYPE;
		virtual std::string getComponentType();
		virtual void onSceneLoaded();
		virtual void onStateChanged();
		virtual Component* onClone();
		virtual void onRebindObject(std::string oldObj, std::string newObj);
		virtual void onRefresh();

		std::string getConnectedObjectGuid() { return connectedObjectGuid; }
		void setConnectedObjectGuid(std::string name);

		glm::vec3 getAnchor() { return anchor; }
		void setAnchor(glm::vec3 value) { anchor = value; }

		glm::vec3 getConnectedAnchor() { return connectedAnchor; }
		void setConnectedAnchor(glm::vec3 value) { connectedAnchor = value; }

		bool getLinkedBodiesCollision() { return linkedBodiesCollision; }
		void setLinkedBodiesCollision(bool value) { linkedBodiesCollision = value; }

		void autoConfigureAnchors();
	};
}