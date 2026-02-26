#pragma once

#include "ComponentEditor.h"

#include "../Engine/glm/vec3.hpp"

namespace GX
{
	class Property;
	class PropVector3;

	class TransformEditor : public ComponentEditor
	{
	public:
		TransformEditor();
		~TransformEditor();

		virtual void init(std::vector<Component*> comps);

		void updateTransform();

	private:
		void onChangePosition(Property* prop, glm::vec3 val);
		void onChangeRotation(Property* prop, glm::vec3 val);
		void onChangeScale(Property* prop, glm::vec3 val);

		PropVector3* position = nullptr;
		PropVector3* rotation = nullptr;
		PropVector3* scale = nullptr;
	};
}