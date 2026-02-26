#pragma once

#include "ComponentEditor.h"

#include "../Engine/glm/vec2.hpp"
#include "../Engine/glm/vec3.hpp"

namespace GX
{
	class Property;
	class TreeNode;
	class GameObject;

	class ConeTwistJointEditor : public ComponentEditor
	{
	public:
		ConeTwistJointEditor();
		~ConeTwistJointEditor();

		virtual void init(std::vector<Component*> comps);

	private:
		void onChangeLinkedBodiesCollision(Property* prop, bool val);
		void onChangeAnchor(Property* prop, glm::vec3 val);
		void onChangeConnectedAnchor(Property* prop, glm::vec3 val);
		void onChangeLimits(Property* prop, glm::vec3 val);

		void onDropConnectedBody(TreeNode* prop, TreeNode* from);
		void onClickAutoConfigure(Property* prop);
	};
}