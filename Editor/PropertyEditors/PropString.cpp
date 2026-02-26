#include "PropString.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <dear-imgui/misc/cpp/imgui_stdlib.h>

namespace GX
{
	PropString::PropString(PropertyEditor* ed, std::string name, std::string val) : Property(ed, name)
	{
		setValue(val);

		flags = ImGuiTreeNodeFlags_AllowItemOverlap;
		fullSpaceWidth = false;

		guid1 = "##" + genGuid();
	}

	PropString::~PropString()
	{
	}

	void PropString::update(bool opened)
	{
		Property::update(opened);

		//ImGui::SameLine();

		int flags = 0;

		if (readOnly)
			flags = ImGuiInputTextFlags_ReadOnly;

		ImGui::PushItemWidth(-1);
		if (multiline)
		{
			ImGui::InputTextMultiline(guid1.c_str(), &value, ImVec2(0, 0), flags);
			if (ImGui::IsItemDeactivatedAfterEdit())
			{
				if (onChangeCallback != nullptr)
				{
					onChangeCallback(this, value);
				}
			}
		}
		else
		{
			ImGui::InputText(guid1.c_str(), &value, flags);
			if (ImGui::IsItemDeactivatedAfterEdit())
			{
				if (onChangeCallback != nullptr)
				{
					onChangeCallback(this, value);
				}
			}
		}
		ImGui::PopItemWidth();
	}

	void PropString::setValue(std::string val)
	{
		value = val;
	}
}