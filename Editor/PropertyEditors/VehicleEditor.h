#pragma once

#include "ComponentEditor.h"

#include "../Engine/glm/vec3.hpp"

#include <string>

namespace GX
{
	class Property;
	class TreeNode;
	class GameObject;

	class VehicleEditor : public ComponentEditor
	{
	public:
		VehicleEditor();
		~VehicleEditor();

		virtual void init(std::vector<Component*> comps);

	private:
		void onChangeAxis(Property* prop, std::string val);
		void onChangeInvertForward(Property* prop, bool val);
		
		void onDropConnectedBody(TreeNode* prop, TreeNode* from);
		void onClickAutoConfigure(Property* prop);

		void onChangeRadius(TreeNode* prop, float value);
		void onChangeWidth(TreeNode* prop, float value);
		void onChangeSuspensionStiffness(TreeNode* prop, float value);
		void onChangeSuspensionDamping(TreeNode* prop, float value);
		void onChangeSuspensionCompression(TreeNode* prop, float value);
		void onChangeSuspensionRestLength(TreeNode* prop, float value);
		void onChangeFriction(TreeNode* prop, float value);
		void onChangeRollInfluence(TreeNode* prop, float value);
		void onChangeDirection(TreeNode* prop, glm::vec3 value);
		void onChangeAxle(TreeNode* prop, glm::vec3 value);
		void onChangeConnectionPoint(TreeNode* prop, glm::vec3 value);
		void onChangeIsFrontWheel(TreeNode* prop, bool value);

		void onClickAddWheel(TreeNode* prop);
		void onClickWheelPopup(TreeNode* prop, int index);
	};
}