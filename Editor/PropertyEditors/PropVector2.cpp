#include "PropertyEditor.h"
#include "PropVector2.h"

#include <imgui.h>
#include <imgui_internal.h>
#include "../../Engine/glm/gtc/type_ptr.hpp"

namespace GX
{
	PropVector2::PropVector2(PropertyEditor* ed, std::string name, glm::vec2 val) : Property(ed, name)
	{
		setValue(val);

		flags = ImGuiTreeNodeFlags_AllowItemOverlap;
		fullSpaceWidth = false;
	}

	PropVector2::~PropVector2()
	{
	}

	void PropVector2::update(bool opened)
	{
		Property::update(opened);

		//ImGui::SameLine();
		ImGui::PushItemWidth(-1);
		if (isDraggable)
		{
			bool b = false;
			if (type == ValueType::Float)
				b = ImGui::DragFloat2("", glm::value_ptr(value), 0.05f, 0, 0, "%.6f");
			else
				b = ImGui::DragInt2("", glm::value_ptr(valueInt));

			//if (b)
			if (ImGui::IsItemDeactivatedAfterEdit())
			{
				if (onChangeCallback != nullptr)
				{
					if (type == ValueType::Float)
						onChangeCallback(this, value);
					else
						onChangeCallback(this, valueInt);
				}
			}
		}
		else
		{
			bool b = false;
			if (type == ValueType::Float)
				b = ImGui::InputFloat2("", glm::value_ptr(value), "%.6f");
			else
				b = ImGui::InputInt2("", glm::value_ptr(valueInt));

			//if (b)
			if (ImGui::IsItemDeactivatedAfterEdit())
			{
				if (onChangeCallback != nullptr)
				{
					if (type == ValueType::Float)
						onChangeCallback(this, value);
					else
						onChangeCallback(this, valueInt);
				}
			}
		}
		ImGui::PopItemWidth();
	}

	void PropVector2::setValue(glm::vec2 val)
	{
		value = val;
		valueInt = val;
	}
}