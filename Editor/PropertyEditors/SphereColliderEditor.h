#pragma once

#include "ComponentEditor.h"

#include "../Engine/glm/vec3.hpp"

namespace GX
{
	class Property;
	class GameObject;

	class SphereColliderEditor : public ComponentEditor
	{
	public:
		SphereColliderEditor();
		~SphereColliderEditor();

		virtual void init(std::vector<Component*> comps);

	private:
		void onChangeRadius(Property* prop, float val);
		void onChangeOffsetPosition(Property* prop, glm::vec3 val);
		void onChangeIsTrigger(Property* prop, bool val);
	};
}