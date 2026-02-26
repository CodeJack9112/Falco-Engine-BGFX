#include "PropEditorHost.h"
#include "PropertyEditor.h"
#include "../Classes/TreeView.h"

#include <imgui.h>
#include <dear-imgui/misc/cpp/imgui_stdlib.h>
#include <imgui_internal.h>

namespace GX
{
	PropEditorHost::PropEditorHost(PropertyEditor* ed, std::string name, PropertyEditor* hosted) : Property(ed, name)
	{
		setClassName("PropEditorHost");

		setHostedEditor(hosted);
	}

	PropEditorHost::~PropEditorHost()
	{
		if (hostedEditor != nullptr)
			delete hostedEditor;

		hostedEditor = nullptr;
	}

	void PropEditorHost::update(bool opened)
	{
		Property::update(opened);

		if (opened)
		{
			if (hostedEditor != nullptr)
				hostedEditor->update();
		}
	}

	void PropEditorHost::setHostedEditor(PropertyEditor* hosted)
	{
		hostedEditor = hosted;
		hostedEditor->setParentEditor(getEditor(), this);

		if (hostedEditor != nullptr)
			alwaysShowArrow = true;
		else
			alwaysShowArrow = false;
	}
}