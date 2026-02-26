#include "PropButton2.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <dear-imgui/misc/cpp/imgui_stdlib.h>

#include "../Engine/Assets/Texture.h"

namespace GX
{
	PropButton2::PropButton2(PropertyEditor* ed, std::string name, std::string val1, std::string val2) : Property(ed, name)
	{
		setValue1(val1);
		setValue2(val2);

		flags = ImGuiTreeNodeFlags_AllowItemOverlap;
		fullSpaceWidth = false;
	}

	PropButton2::~PropButton2()
	{
	}

	void PropButton2::update(bool opened)
	{
		Property::update(opened);

		ImGui::PushItemWidth(-1);
		
		ImVec2 sz = ImGui::GetContentRegionAvail();

		bool btn1 = false;
		bool btn2 = false;

		if (image1 == nullptr)
			btn1 = ImGui::Button(value1.c_str(), ImVec2(sz.x / 2 - 4, 0));
		else
			btn1 = ImGui::ImageButton((void*)image1->getHandle().idx, ImVec2(32, 36));

		if (btn1)
		{
			if (onClickCallback != nullptr)
			{
				onClickCallback(this, 0);
			}
		}

		ImGui::SameLine();

		if (image2 == nullptr)
			btn2 = ImGui::Button(value2.c_str(), ImVec2(sz.x / 2 - 4, 0));
		else
			btn2 = ImGui::ImageButton((void*)image2->getHandle().idx, ImVec2(32, 36));

		if (btn2)
		{
			if (onClickCallback != nullptr)
			{
				onClickCallback(this, 1);
			}
		}

		ImGui::PopItemWidth();
	}

	void PropButton2::setValue1(std::string val)
	{
		value1 = val;
	}

	void PropButton2::setValue2(std::string val)
	{
		value2 = val;
	}
}