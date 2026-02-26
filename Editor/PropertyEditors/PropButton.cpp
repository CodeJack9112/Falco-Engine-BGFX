#include "PropButton.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <dear-imgui/misc/cpp/imgui_stdlib.h>

#include "../Engine/Assets/Texture.h"
#include "../Engine/UI/ImGUIWidgets.h"

namespace GX
{
	PropButton::PropButton(PropertyEditor* ed, std::string name, std::string val) : Property(ed, name)
	{
		setValue(val);

		flags = ImGuiTreeNodeFlags_AllowItemOverlap;
		fullSpaceWidth = false;
	}

	PropButton::~PropButton()
	{
	}

	void PropButton::update(bool opened)
	{
		Property::update(opened);

		//ImGui::PushItemWidth(-1);
		ImVec2 sz = ImGui::GetContentRegionAvail();

		bool btn = false;

		if (image == nullptr)
			btn = ImGui::Button(value.c_str(), ImVec2(sz.x, 0));
		else
			btn = ImGui::ImageButtonWithText((void*)image->getHandle().idx, value.c_str(), ImVec2(16, 16), ImVec2(0, 1), ImVec2(1, 0), ImVec2(sz.x, -1.0f));

		if (btn)
		{
			if (onClickCallback != nullptr)
			{
				onClickCallback(this);
			}
		}
		//ImGui::PopItemWidth();
	}

	void PropButton::setValue(std::string val)
	{
		value = val;
	}
}