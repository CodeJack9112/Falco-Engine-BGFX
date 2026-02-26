#pragma once

#include "ComponentEditor.h"

namespace GX
{
	class Property;
	class GameObject;

	class MeshColliderEditor : public ComponentEditor
	{
	public:
		MeshColliderEditor();
		~MeshColliderEditor();

		virtual void init(std::vector<Component*> comps);

	private:
		void onChangeConvex(Property* prop, bool val);
		void onChangeIsTrigger(Property* prop, bool val);
	};
}