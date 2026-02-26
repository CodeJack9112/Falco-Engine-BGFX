#include "PropCustom.h"

#include <imgui.h>
#include <imgui_internal.h>

namespace GX
{
	PropCustom::PropCustom(PropertyEditor* ed, std::string name) : Property(ed, name)
	{
		flags = ImGuiTreeNodeFlags_AllowItemOverlap;
		fullSpaceWidth = false;

		guid1 = "##" + genGuid();

		skipNode = true;
	}

	PropCustom::~PropCustom()
	{
	}

	void PropCustom::update(bool opened)
	{
		//__super::update(opened);

		ImGui::PushItemWidth(-1);
		
		if (onUpdateCallback != nullptr)
			onUpdateCallback(this);

		ImGui::PopItemWidth();
	}
}