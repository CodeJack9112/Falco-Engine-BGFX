#pragma once

#include "Collider.h"

#include "../glm/vec3.hpp"

namespace GX
{
	class BoxCollider : public Collider
	{
	private:
		glm::vec3 boxSize = glm::vec3(1, 1, 1);

	public:
		BoxCollider();
		virtual ~BoxCollider();

		static std::string COMPONENT_TYPE;
		virtual std::string getComponentType();
		virtual Component* onClone();

		glm::vec3 getBoxSize() { return boxSize; }
		void setBoxSize(glm::vec3 value);
	};
}