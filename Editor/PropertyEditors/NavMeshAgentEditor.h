#pragma once

#include "ComponentEditor.h"

namespace GX
{
	class Property;
	class GameObject;

	class NavMeshAgentEditor : public ComponentEditor
	{
	public:
		NavMeshAgentEditor();
		~NavMeshAgentEditor();

		virtual void init(std::vector<Component*> comps);

	private:
		void onChangeRadius(Property* prop, float val);
		void onChangeHeight(Property* prop, float val);
		void onChangeSpeed(Property* prop, float val);
		void onChangeAcceleration(Property* prop, float val);
		void onChangeRotationSpeed(Property* prop, float val);
	};
}