#pragma once

#include "ComponentEditor.h"

#include "../Engine/glm/vec2.hpp"
#include "../Engine/glm/vec3.hpp"

namespace GX
{
	class Property;
	class TreeNode;
	class GameObject;

	class HingeJointEditor : public ComponentEditor
	{
	public:
		HingeJointEditor();
		~HingeJointEditor();

		virtual void init(std::vector<Component*> comps);

	private:
		void onChangeLinkedBodiesCollision(Property* prop, bool val);
		void onChangeAnchor(Property* prop, glm::vec3 val);
		void onChangeConnectedAnchor(Property* prop, glm::vec3 val);
		void onChangeAxis(Property* prop, glm::vec3 val);
		void onChangeLimits(Property* prop, glm::vec2 val);

		void onDropConnectedBody(TreeNode* prop, TreeNode* from);
		void onClickAutoConfigure(Property* prop);
	};
}