#pragma once

#include "ComponentEditor.h"

#include "../Engine/glm/vec3.hpp"

namespace GX
{
	class Property;
	class GameObject;

	class TerrainColliderEditor : public ComponentEditor
	{
	public:
		TerrainColliderEditor();
		~TerrainColliderEditor();

		virtual void init(std::vector<Component*> comps);

	private:
		void onChangeIsTrigger(Property* prop, bool val);
	};
}