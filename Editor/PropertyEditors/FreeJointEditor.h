#pragma once

#include "ComponentEditor.h"

#include "../Engine/glm/vec3.hpp"

namespace GX
{
	class Property;
	class TreeNode;
	class GameObject;

	class FreeJointEditor : public ComponentEditor
	{
	public:
		FreeJointEditor();
		~FreeJointEditor();

		virtual void init(std::vector<Component*> comps);

	private:
		void onChangeLinkedBodiesCollision(Property* prop, bool val);
		void onChangeAnchor(Property* prop, glm::vec3 val);
		void onChangeConnectedAnchor(Property* prop, glm::vec3 val);
		void onChangeLimitMin(Property* prop, glm::vec3 val);
		void onChangeLimitMax(Property* prop, glm::vec3 val);

		void onDropConnectedBody(TreeNode* prop, TreeNode* from);
		void onClickAutoConfigure(Property* prop);
	};
}