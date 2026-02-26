#include "PropInfo.h"

#include <imgui.h>
#include <imgui_internal.h>

#include "../Engine/Assets/Texture.h"

namespace GX
{
	PropInfo::PropInfo(PropertyEditor* ed, std::string name, std::string txt) : Property(ed, name)
	{
		flags = ImGuiTreeNodeFlags_AllowItemOverlap;
		fullSpaceWidth = false;

		guid1 = "##" + genGuid();

		text = txt;

		skipNode = true;
	}

	PropInfo::~PropInfo()
	{
	}

	void PropInfo::update(bool opened)
	{
		ImGui::PushItemWidth(-1);

		ImGui::BeginGroup();
		float w = ImGui::GetContentRegionAvail().x;
		ImVec2 cp = ImGui::GetCursorPos();
		ImGui::SetCursorPos(ImVec2(cp.x + 10, cp.y + 10));
		ImGui::Image((void*)icon->getHandle().idx, ImVec2(16, 16), ImVec2(0, 1), ImVec2(1, 0));
		ImGui::SameLine();

		ImGui::TextWrapped(text.c_str());

		ImGui::Dummy(ImVec2(10, 10));
		ImGui::EndGroup();
		ImVec2 rm = ImVec2(ImGui::GetItemRectMin().x + w, ImGui::GetItemRectMax().y);
		ImVec4 col = ImGui::GetStyle().Colors[ImGuiCol_Border];
		ImGui::GetForegroundDrawList()->AddRect(ImGui::GetItemRectMin(), rm, IM_COL32(col.x * 255, col.y * 255, col.z * 255, col.w * 255), 4.0f);

		ImGui::PopItemWidth();
	}
}