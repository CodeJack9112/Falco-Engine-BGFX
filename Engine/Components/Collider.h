#pragma once

#include "Component.h"
#include <string>

#include "../glm/vec3.hpp"
#include "../glm/gtx/quaternion.hpp"

class btCollisionShape;

namespace GX
{
	class Collider : public Component
	{
	protected:
		glm::vec3 offset = glm::vec3(0, 0, 0);
		glm::vec3 scale = glm::vec3(1, 1, 1);
		glm::highp_quat rotation = glm::identity<glm::highp_quat>();
		bool isTrigger = false;

		void reloadBody();

	public:
		Collider(MonoClass * monoClass) : Component(monoClass) {}
		virtual ~Collider();

		btCollisionShape* collisionShape = nullptr;

		void setCollisionShape(btCollisionShape* shape);
		btCollisionShape * getCollisionShape() { return collisionShape; }

		glm::vec3 getOffset() { return offset; }
		void setOffset(glm::vec3 value);

		glm::vec3 getScale() { return scale; }

		glm::highp_quat getRotation() { return rotation; }
		void setRotation(glm::highp_quat value);

		bool getIsTrigger() { return isTrigger; }
		void setIsTrigger(bool value);

		virtual void onAttach();
		virtual void onStateChanged();
		virtual void onSceneLoaded();
	};
}