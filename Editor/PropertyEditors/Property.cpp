#include "PropertyEditor.h"
#include "Property.h"

#include <imgui.h>

namespace GX
{
	Property::Property(PropertyEditor* ed, std::string name) : TreeNode(ed->getTreeView())
	{
		setClassName("Property");

		genName(name);
		editor = ed;
		this->expanded = true;

		flags = ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_Framed;
		setEditable(false);
	}

	Property::~Property()
	{
		
	}

	void Property::update(bool opened)
	{
		TreeNode::update(opened);
	}

	void Property::callClearCallback()
	{
		if (clearCallback != nullptr)
			clearCallback(this);
	}
}