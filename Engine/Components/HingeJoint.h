#pragma once

#include "Component.h"

#include "../glm/vec3.hpp"
#include "../glm/gtc/quaternion.hpp"

#include <string>

class btHingeConstraint;

namespace GX
{
	class HingeJoint : public Component
	{
	private:
		std::string connectedObjectGuid = "[None]";
		glm::vec3 anchor = glm::vec3(0, 0, 0);
		glm::vec3 connectedAnchor = glm::vec3(0, 0, 0);
		glm::vec3 axis = glm::vec3(1, 0, 0);
		float limitMin = -3.14f;
		float limitMax = 3.14f;
		bool linkedBodiesCollision = true;

		btHingeConstraint * hingeConstraint = nullptr;

		void removeConstraint();

	public:
		HingeJoint();
		virtual ~HingeJoint();

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

		glm::vec3 getAxis() { return axis; }
		void setAxis(glm::vec3 value) { axis = value; }

		float getLimitMin() { return limitMin; }
		void setLimitMin(float value) { limitMin = value; }

		float getLimitMax() { return limitMax; }
		void setLimitMax(float value) { limitMax = value; }

		bool getLinkedBodiesCollision() { return linkedBodiesCollision; }
		void setLinkedBodiesCollision(bool value) { linkedBodiesCollision = value; }

		void autoConfigureAnchors();
	};
}