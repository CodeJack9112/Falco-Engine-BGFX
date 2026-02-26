#pragma once

#include "../Engine/glm/vec3.hpp"

#include "ComponentEditor.h"
#include "../Engine/Renderer/Color.h"

namespace GX
{
	class Texture;

	class Property;
	class PropFloat;

	class DecalRendererEditor : public ComponentEditor
	{
	public:
		DecalRendererEditor();
		~DecalRendererEditor();

		virtual void init(std::vector<Component*> comps);

	private:
		void onChangeMaterial(TreeNode* prop, TreeNode* from);
		void onClearMaterial(Property* prop);
	};
}