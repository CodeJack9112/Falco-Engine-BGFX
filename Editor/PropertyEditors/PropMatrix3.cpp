#include "PropMatrix3.h"

#include <imgui.h>
#include <imgui_internal.h>
#include "../../Engine/glm/gtc/type_ptr.hpp"

namespace GX
{
	PropMatrix3::PropMatrix3(PropertyEditor* ed, std::string name, glm::mat3x3 val) : Property(ed, name)
	{
		setValue(val);

		flags = ImGuiTreeNodeFlags_AllowItemOverlap;
		fullSpaceWidth = false;

		guid1 = "##" + genGuid();
		guid2 = "##" + genGuid();
		guid3 = "##" + genGuid();
	}

	PropMatrix3::~PropMatrix3()
	{
	}

	void PropMatrix3::update(bool opened)
	{
		Property::update(opened);

		ImGui::PushItemWidth(-1);
		ImGui::InputFloat3(guid1.c_str(), glm::value_ptr(value[0]), "%.6f");
		if (ImGui::IsItemDeactivatedAfterEdit())
		{
			if (onChangeCallback != nullptr)
				onChangeCallback(this, value);
		}

		ImGui::InputFloat3(guid2.c_str(), glm::value_ptr(value[1]), "%.6f");
		if (ImGui::IsItemDeactivatedAfterEdit())
		{
			if (onChangeCallback != nullptr)
				onChangeCallback(this, value);
		}

		ImGui::InputFloat3(guid3.c_str(), glm::value_ptr(value[2]), "%.6f");
		if (ImGui::IsItemDeactivatedAfterEdit())
		{
			if (onChangeCallback != nullptr)
				onChangeCallback(this, value);
		}
		ImGui::PopItemWidth();
	}

	void PropMatrix3::setValue(glm::mat3x3 val)
	{
		value = val;
	}
}