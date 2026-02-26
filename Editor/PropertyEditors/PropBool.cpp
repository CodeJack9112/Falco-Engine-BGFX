#include "PropBool.h"

#include <imgui.h>
#include <imgui_internal.h>

namespace GX
{
	PropBool::PropBool(PropertyEditor* ed, std::string name, bool val) : Property(ed, name)
	{
		setValue(val);

		flags = ImGuiTreeNodeFlags_AllowItemOverlap;
		fullSpaceWidth = false;

		guid1 = "##" + genGuid();
	}

	PropBool::~PropBool()
	{
	}

	void PropBool::update(bool opened)
	{
		Property::update(opened);

		//ImGui::SameLine();
		ImGui::PushItemWidth(-1);
		if (ImGui::Checkbox(guid1.c_str(), &value))
		{
			if (onChangeCallback != nullptr)
			{
				onChangeCallback(this, value);
			}
		}
		ImGui::PopItemWidth();
	}

	void PropBool::setValue(bool val)
	{
		value = val;
	}
}