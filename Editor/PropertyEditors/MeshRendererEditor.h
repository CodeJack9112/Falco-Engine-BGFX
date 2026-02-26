#pragma once

#include "ComponentEditor.h"

#include "../Engine/glm/vec3.hpp"

namespace GX
{
	class Property;

	class MeshRendererEditor : public ComponentEditor
	{
	public:
		MeshRendererEditor();
		~MeshRendererEditor();

		virtual void init(std::vector<Component*> comps);

	private:
		bool isMaterialsIdentical();
		void listMaterials();

		void onClickSkinRoot(TreeNode* prop);
		void onChangeCastShadows(Property* prop, bool value);
		void onChangeLightmapSize(Property* prop, int value);
		void onChangeLodMaxDistance(Property* prop, float value);
		void onChangeCullOverMaxDistance(Property* prop, bool value);
		void onDropSkinRoot(TreeNode* prop, TreeNode* from);
		void onDropMaterial(TreeNode* prop, TreeNode* from, int index);
	};
}