#include "PropColorPicker.h"

#include <imgui.h>
#include <imgui_internal.h>

namespace GX
{
	PropColorPicker::PropColorPicker(PropertyEditor* ed, std::string name, Color val) : Property(ed, name)
	{
		setValue(val);

		flags = ImGuiTreeNodeFlags_AllowItemOverlap;
		fullSpaceWidth = false;

		guid1 = "##" + genGuid();
	}

	PropColorPicker::~PropColorPicker()
	{
	}

	void PropColorPicker::update(bool opened)
	{
		Property::update(opened);

		//ImGui::SameLine();
		ImGui::PushItemWidth(-1);
		ImGui::ColorEdit4(guid1.c_str(), value.ptr());
		if (ImGui::IsItemDeactivatedAfterEdit())
		{
			if (onChangeCallback != nullptr)
			{
				onChangeCallback(this, value);
			}
		}
		ImGui::PopItemWidth();
	}

	void PropColorPicker::setValue(Color val)
	{
		value = val;
	}
}