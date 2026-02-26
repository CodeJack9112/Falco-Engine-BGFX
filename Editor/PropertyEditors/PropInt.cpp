#include "PropInt.h"

#include <imgui.h>
#include <imgui_internal.h>

namespace GX
{
	PropInt::PropInt(PropertyEditor* ed, std::string name, int val) : Property(ed, name)
	{
		setValue(val);

		flags = ImGuiTreeNodeFlags_AllowItemOverlap;
		fullSpaceWidth = false;

		guid1 = "##" + genGuid();
	}

	PropInt::~PropInt()
	{
	}

	void PropInt::update(bool opened)
	{
		Property::update(opened);

		//ImGui::SameLine();
		ImGui::PushItemWidth(-1);
		if (minVal != maxVal != 0.0f)
		{
			if (isSlider)
			{
				ImGui::SliderInt(guid1.c_str(), &value, minVal, maxVal, "%d");
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
				int prevVal = value;
				ImGui::InputInt(guid1.c_str(), &value, 1, 100);
				if (ImGui::IsItemDeactivatedAfterEdit())
				{
					if (value >= minVal && value <= maxVal)
					{
						if (onChangeCallback != nullptr)
						{
							onChangeCallback(this, value);
						}
					}
					else
					{
						if (value < minVal) value = minVal;
						if (value > maxVal) value = maxVal;
					}
				}
			}
		}
		else
		{
			if (isDraggable)
			{
				ImGui::DragInt(guid1.c_str(), &value);
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
				ImGui::InputInt(guid1.c_str(), &value, 1, 100);
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

	void PropInt::setValue(int val)
	{
		value = val;
	}
}