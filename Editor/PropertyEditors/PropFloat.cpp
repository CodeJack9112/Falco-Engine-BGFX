#include "PropFloat.h"

#include <imgui.h>
#include <imgui_internal.h>

namespace GX
{
	PropFloat::PropFloat(PropertyEditor* ed, std::string name, float val) : Property(ed, name)
	{
		setValue(val);

		flags = ImGuiTreeNodeFlags_AllowItemOverlap;
		fullSpaceWidth = false;

		guid1 = "##" + genGuid();
	}

	PropFloat::~PropFloat()
	{
	}

	void PropFloat::update(bool opened)
	{
		Property::update(opened);

		//ImGui::SameLine();
		ImGui::PushItemWidth(-1);
		if (minVal != maxVal != 0.0f)
		{
			ImGui::SliderFloat(guid1.c_str(), &value, minVal, maxVal, "%.6f");
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
			if (isDraggable)
			{
				ImGui::DragFloat(guid1.c_str(), &value, 0.05f, 0, 0, "%.6f");
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
				ImGui::InputFloat(guid1.c_str(), &value);
				if (ImGui::IsItemDeactivatedAfterEdit())
				{
					if (onChangeCallback != nullptr)
					{
						onChangeCallback(this, value);
					}
				}
			}
		}
		ImGui::PopItemWidth();
	}

	void PropFloat::setValue(float val)
	{
		value = val;
	}
}