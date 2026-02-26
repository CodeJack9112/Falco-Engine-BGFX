#pragma once

#include "ComponentEditor.h"

#include "../Engine/glm/vec3.hpp"

namespace GX
{
	class Property;
	class GameObject;

	class RigidBodyEditor : public ComponentEditor
	{
	public:
		RigidBodyEditor();
		~RigidBodyEditor();

		virtual void init(std::vector<Component*> comps);

	private:
		void onChangeMass(Property* prop, float val);
		void onChangeFreezePosition(Property* prop, bool val[3]);
		void onChangeFreezeRotation(Property* prop, bool val[3]);
		void onChangeKinematic(Property* prop, bool val);
		void onChangeStatic(Property* prop, bool val);
		void onChangeUseOwnGravity(Property* prop, bool val);
		void onChangeGravity(Property* prop, glm::vec3 val);
		void onChangeFriction(Property* prop, float val);
		void onChangeBounciness(Property* prop, float val);
		void onChangeLinearDamping(Property* prop, float val);
		void onChangeAngularDamping(Property* prop, float val);
	};
}