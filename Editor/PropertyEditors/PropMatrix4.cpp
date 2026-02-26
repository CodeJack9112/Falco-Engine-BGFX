#include "PropMatrix4.h"

#include <imgui.h>
#include <imgui_internal.h>
#include "../../Engine/glm/gtc/type_ptr.hpp"

namespace GX
{
	PropMatrix4::PropMatrix4(PropertyEditor* ed, std::string name, glm::mat4x4 val) : Property(ed, name)
	{
		setValue(val);

		flags = ImGuiTreeNodeFlags_AllowItemOverlap;
		fullSpaceWidth = false;

		guid1 = "##" + genGuid();
		guid2 = "##" + genGuid();
		guid3 = "##" + genGuid();
		guid4 = "##" + genGuid();
	}

	PropMatrix4::~PropMatrix4()
	{
	}

	void PropMatrix4::update(bool opened)
	{
		Property::update(opened);

		ImGui::PushItemWidth(-1);
		ImGui::InputFloat4(guid1.c_str(), glm::value_ptr(value[0]), "%.6f");
		if (ImGui::IsItemDeactivatedAfterEdit())
		{
			if (onChangeCallback != nullptr)
				onChangeCallback(this, value);
		}

		ImGui::InputFloat4(guid2.c_str(), glm::value_ptr(value[1]), "%.6f");
		if (ImGui::IsItemDeactivatedAfterEdit())
		{
			if (onChangeCallback != nullptr)
				onChangeCallback(this, value);
		}

		ImGui::InputFloat4(guid3.c_str(), glm::value_ptr(value[2]), "%.6f");
		if (ImGui::IsItemDeactivatedAfterEdit())
		{
			if (onChangeCallback != nullptr)
				onChangeCallback(this, value);
		}

		ImGui::InputFloat4(guid4.c_str(), glm::value_ptr(value[3]), "%.6f");
		if (ImGui::IsItemDeactivatedAfterEdit())
		{
			if (onChangeCallback != nullptr)
				onChangeCallback(this, value);
		}
		ImGui::PopItemWidth();
	}

	void PropMatrix4::setValue(glm::mat4x4 val)
	{
		value = val;
	}
}