#include "PropertyEditor.h"
#include "PropVector4.h"

#include <imgui.h>
#include <imgui_internal.h>
#include "../../Engine/glm/gtc/type_ptr.hpp"

namespace GX
{
	PropVector4::PropVector4(PropertyEditor* ed, std::string name, glm::vec4 val) : Property(ed, name)
	{
		setValue(val);

		flags = ImGuiTreeNodeFlags_AllowItemOverlap;
		fullSpaceWidth = false;
	}

	PropVector4::~PropVector4()
	{
	}

	void PropVector4::update(bool opened)
	{
		Property::update(opened);

		//ImGui::SameLine();
		ImGui::PushItemWidth(-1);
		if (isDraggable)
		{
			ImGui::DragFloat4("", glm::value_ptr(value), 0.05f, 0, 0, "%.6f");
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
			ImGui::InputFloat4("", glm::value_ptr(value), "%.6f");
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

	void PropVector4::setValue(glm::vec4 val)
	{
		value = val;
	}
}