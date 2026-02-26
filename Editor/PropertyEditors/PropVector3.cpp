#include "PropertyEditor.h"
#include "PropVector3.h"

#include <imgui.h>
#include <imgui_internal.h>

#include "../../Engine/glm/gtc/type_ptr.hpp"
#include "../../Engine/UI/ImGUIWidgets.h"

namespace GX
{
	PropVector3::PropVector3(PropertyEditor* ed, std::string name, glm::vec3 val) : Property(ed, name)
	{
		setValue(val);

		flags = ImGuiTreeNodeFlags_AllowItemOverlap;
		fullSpaceWidth = false;
	}

	PropVector3::~PropVector3()
	{
		
	}

	void PropVector3::update(bool opened)
	{
		Property::update(opened);

		std::string _maskX = "%.6f";
		std::string _maskY = "%.6f";
		std::string _maskZ = "%.6f";

		if (showBadge)
		{
			if (getNumberOfZeroes(value.x) == 0) _maskX = "%.6f";
			if (getNumberOfZeroes(value.x) == 1) _maskX = "%.5f";
			if (getNumberOfZeroes(value.x) == 2) _maskX = "%.4f";
			if (getNumberOfZeroes(value.x) == 3) _maskX = "%.3f";
			if (getNumberOfZeroes(value.x) == 4) _maskX = "%.2f";
			if (getNumberOfZeroes(value.x) == 5) _maskX = "%.1f";
			if (getNumberOfZeroes(value.x) == 6) _maskX = "%.1f";

			if (getNumberOfZeroes(value.y) == 0) _maskY = "%.6f";
			if (getNumberOfZeroes(value.y) == 1) _maskY = "%.5f";
			if (getNumberOfZeroes(value.y) == 2) _maskY = "%.4f";
			if (getNumberOfZeroes(value.y) == 3) _maskY = "%.3f";
			if (getNumberOfZeroes(value.y) == 4) _maskY = "%.2f";
			if (getNumberOfZeroes(value.y) == 5) _maskY = "%.1f";
			if (getNumberOfZeroes(value.y) == 6) _maskY = "%.1f";

			if (getNumberOfZeroes(value.z) == 0) _maskZ = "%.6f";
			if (getNumberOfZeroes(value.z) == 1) _maskZ = "%.5f";
			if (getNumberOfZeroes(value.z) == 2) _maskZ = "%.4f";
			if (getNumberOfZeroes(value.z) == 3) _maskZ = "%.3f";
			if (getNumberOfZeroes(value.z) == 4) _maskZ = "%.2f";
			if (getNumberOfZeroes(value.z) == 5) _maskZ = "%.1f";
			if (getNumberOfZeroes(value.z) == 6) _maskZ = "%.1f";

			if (!maskX.empty())
				_maskX = _maskX + " " + maskX;

			if (!maskY.empty())
				_maskY = _maskY + " " + maskY;

			if (!maskZ.empty())
				_maskZ = _maskZ + " " + maskZ;
		}

		bool changed = false;

		if (showBadge)
		{
			float w = ImGui::GetContentRegionAvail().x;
			bool wide = w > 160.0f;

			float inputW = w / 3.0f - 4.0f * 3.0f;
			if (wide)
				inputW = w / 3.0f - 6.5f * 3.0f;

			inputW = std::max(0.0f, inputW);

			//X
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.05f, 0, 1));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.8f, 0.05f, 0, 1));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.8f, 0.05f, 0, 1));
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(2, -1));
			ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 2.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 2));

			if (!wide)
				ImGui::Button("##X", ImVec2(4, 0));
			else
				ImGui::Button("X", ImVec2(12, 0));
			ImGui::PopStyleColor(3);
			ImGui::SameLine();
			ImGui::PopStyleVar(3);
			ImGui::SetNextItemWidth(inputW);
			if (isDraggable)
				ImGui::DragFloat(("##X_" + guid1).c_str(), &value.x, 0.05f, 0, 0, _maskX.c_str(), 0);
			else
				ImGui::InputFloat(("##X_" + guid1).c_str(), &value.x, 0.05f, 0.5f, _maskX.c_str());
			ImGui::SameLine();

			if (!changed)
				changed = ImGui::IsItemDeactivatedAfterEdit();

			//Y
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.7f, 0, 1));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.7f, 0, 1));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.2f, 0.7f, 0, 1));
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(2, -1));
			ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 2.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 2));

			if (!wide)
				ImGui::Button("##Y", ImVec2(4, 0));
			else
				ImGui::Button("Y", ImVec2(12, 0));
			ImGui::PopStyleColor(3);
			ImGui::SameLine();
			ImGui::PopStyleVar(3);
			ImGui::SetNextItemWidth(inputW);
			if (isDraggable)
				ImGui::DragFloat(("##Y_" + guid1).c_str(), &value.y, 0.05f, 0, 0, _maskY.c_str(), 0);
			else
				ImGui::InputFloat(("##Y_" + guid1).c_str(), &value.y, 0.05f, 0.5f, _maskY.c_str());
			ImGui::SameLine();

			if (!changed)
				changed = ImGui::IsItemDeactivatedAfterEdit();

			//Z
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.20f, 0.41f, 0.92f, 1));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.20f, 0.41f, 0.92f, 1));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.20f, 0.41f, 0.92f, 1));
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(2, -1));
			ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 2.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 2));

			if (!wide)
				ImGui::Button("##Z", ImVec2(4, 0));
			else
				ImGui::Button("Z", ImVec2(12, 0));

			ImGui::PopStyleColor(3);
			ImGui::SameLine();
			ImGui::PopStyleVar(3);
			ImGui::SetNextItemWidth(inputW);
			if (isDraggable)
				ImGui::DragFloat(("##Z" + guid1).c_str(), &value.z, 0.05f, 0, 0, _maskZ.c_str(), 0);
			else
				ImGui::InputFloat(("##Z_" + guid1).c_str(), &value.z, 0.05f, 0.5f, _maskZ.c_str());

			if (!changed)
				changed = ImGui::IsItemDeactivatedAfterEdit();
		}
		else
		{
			ImGui::PushItemWidth(-1);
			if (isDraggable)
				ImGui::DragFloat3(("##" + guid1).c_str(), glm::value_ptr(value), 0.05f, 0, 0, _maskX.c_str(), 0);
			else
				ImGui::InputFloat3(("##" + guid1).c_str(), glm::value_ptr(value), _maskX.c_str());
			ImGui::PopItemWidth();

			changed = ImGui::IsItemDeactivatedAfterEdit();
		}

		if (changed)
		{
			if (onChangeCallback != nullptr)
			{
				onChangeCallback(this, value);
			}
		}
	}

	void PropVector3::setValue(glm::vec3 val)
	{
		value = val;
	}

	int PropVector3::getNumberOfZeroes(float value)
	{
		std::string _mX = std::to_string(value);
		int numZeroes = 0;
		for (int i = _mX.length() - 1; i >= 0; --i)
		{
			if (_mX[i] != '0')
				break;

			++numZeroes;
		}

		return numZeroes;
	}
}