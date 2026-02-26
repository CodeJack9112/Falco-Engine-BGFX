#pragma once

#include "Component.h"
#include <string>

#include "../glm/vec3.hpp"

class btGeneric6DofConstraint;
class btConeTwistConstraint;

namespace GX
{
	class FreeJoint : public Component
	{
	private:
		std::string connectedObjectGuid = "[None]";
		glm::vec3 anchor = glm::vec3(0, 0, 0);
		glm::vec3 connectedAnchor = glm::vec3(0, 0, 0);
		glm::vec3 limitMin = glm::vec3(-3.14, -3.14 * 0.5f, -3.14);
		glm::vec3 limitMax = glm::vec3(3.14, 3.14 * 0.5f, 3.14);
		bool linkedBodiesCollision = true;

		btGeneric6DofConstraint * freeConstraint = nullptr;

		void removeConstraint();

	public:
		FreeJoint();
		virtual ~FreeJoint();

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

		glm::vec3 getLimitMin() { return limitMin; }
		void setLimitMin(glm::vec3 value);

		glm::vec3 getLimitMax() { return limitMax; }
		void setLimitMax(glm::vec3 value);

		bool getLinkedBodiesCollision() { return linkedBodiesCollision; }
		void setLinkedBodiesCollision(bool value) { linkedBodiesCollision = value; }

		void autoConfigureAnchors();
	};
}