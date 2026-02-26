#pragma once

#include "ComponentEditor.h"

#include "../Engine/glm/vec3.hpp"

namespace GX
{
	class Property;
	class GameObject;

	class NavMeshObstacleEditor : public ComponentEditor
	{
	public:
		NavMeshObstacleEditor();
		~NavMeshObstacleEditor();

		virtual void init(std::vector<Component*> comps);

	private:
		void onChangeSize(Property* prop, glm::vec3 val);
		void onChangeOffset(Property* prop, glm::vec3 val);
	};
}