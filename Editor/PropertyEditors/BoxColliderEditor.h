#pragma once

#include "ComponentEditor.h"

#include "../Engine/glm/vec3.hpp"

namespace GX
{
	class Property;
	class GameObject;

	class BoxColliderEditor : public ComponentEditor
	{
	public:
		BoxColliderEditor();
		~BoxColliderEditor();

		virtual void init(std::vector<Component*> comps);

	private:
		void onChangeSize(Property* prop, glm::vec3 val);
		void onChangeOffsetPosition(Property* prop, glm::vec3 val);
		void onChangeOffsetRotation(Property* prop, glm::vec3 val);
		void onChangeIsTrigger(Property* prop, bool val);
	};
}