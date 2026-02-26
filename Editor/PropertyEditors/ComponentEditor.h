#pragma once

#include "PropertyEditor.h"

namespace GX
{
	class GameObject;
	class Component;

	class ComponentEditor : public PropertyEditor
	{
	public:
		ComponentEditor();
		virtual ~ComponentEditor() {}

		virtual void init(std::vector<Component*> comps) { components = comps; }
		virtual void updateEditor();

		std::vector<Component*> & getComponents() { return components; }

	private:
		bool updateState = false;

		void onTreeViewEndUpdate();

	protected:
		std::vector<Component*> components;
	}; 
}