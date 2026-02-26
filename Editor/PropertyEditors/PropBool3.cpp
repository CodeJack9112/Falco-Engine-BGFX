#include "PropBool3.h"

#include <imgui.h>
#include <imgui_internal.h>

namespace GX
{
	PropBool3::PropBool3(PropertyEditor* ed, std::string name, std::string lbl1, std::string lbl2, std::string lbl3, bool * val) : Property(ed, name)
	{
		if (val != nullptr)
			setValue(val);
		else
			memset(value, false, 3);

		flags = ImGuiTreeNodeFlags_AllowItemOverlap;
		fullSpaceWidth = false;

		guid1 = "##" + genGuid();
		guid2 = "##" + genGuid();
		guid3 = "##" + genGuid();

		label1 = lbl1;
		label2 = lbl2;
		label3 = lbl3;
	}

	PropBool3::~PropBool3()
	{
	}

	void PropBool3::update(bool opened)
	{
		Property::update(opened);

		//ImGui::SameLine();
		ImGui::Text(label1.c_str());
		ImGui::SameLine();
		if (ImGui::Checkbox(guid1.c_str(), &value[0]))
		{
			if (onChangeCallback != nullptr)
			{
				onChangeCallback(this, value);
			}
		}
		ImGui::SameLine();
		ImGui::Text(label2.c_str());
		ImGui::SameLine();
		if (ImGui::Checkbox(guid2.c_str(), &value[1]))
		{
			if (onChangeCallback != nullptr)
			{
				onChangeCallback(this, value);
			}
		}
		ImGui::SameLine();
		ImGui::Text(label3.c_str());
		ImGui::SameLine();
		if (ImGui::Checkbox(guid3.c_str(), &value[2]))
		{
			if (onChangeCallback != nullptr)
			{
				onChangeCallback(this, value);
			}
		}
	}

	void PropBool3::setValue(bool val[3])
	{
		value[0] = val[0];
		value[1] = val[1];
		value[2] = val[2];
	}
}