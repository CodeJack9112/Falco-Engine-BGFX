#include "PropSampler.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <dear-imgui/misc/cpp/imgui_stdlib.h>

#include "../Engine/Assets/Texture.h"

namespace GX
{
	PropSampler::PropSampler(PropertyEditor* ed, std::string name, Texture* val) : Property(ed, name)
	{
		setValue(val);

		flags = ImGuiTreeNodeFlags_AllowItemOverlap;
		fullSpaceWidth = false;
	}

	PropSampler::~PropSampler()
	{
	}

	void PropSampler::update(bool opened)
	{
		Property::update(opened);

		ImGui::PushItemWidth(-1);
		ImVec2 sz = ImGui::GetContentRegionAvail();

		bool btn = false;

		if (value == nullptr)
			btn = ImGui::Button("", ImVec2(48, 48));
		else
			btn = ImGui::ImageButton((void*)value->getHandle().idx, ImVec2(38, 44), ImVec2(0, 1), ImVec2(1, 0));

		if (btn)
		{
			if (onClickCallback != nullptr)
			{
				onClickCallback(this);
			}
		}
		ImGui::PopItemWidth();
	}

	void PropSampler::setValue(Texture * val)
	{
		value = val;
	}
}