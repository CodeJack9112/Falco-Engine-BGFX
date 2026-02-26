#include "PropGameObject.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <dear-imgui/misc/cpp/imgui_stdlib.h>

#include "../Engine/Core/GameObject.h"
#include "../Engine/Assets/Texture.h"
#include "../Engine/UI/ImGUIWidgets.h"

namespace GX
{
	PropGameObject::PropGameObject(PropertyEditor* ed, std::string name, GameObject* val) : Property(ed, name)
	{
		setValue(val);

		flags = ImGuiTreeNodeFlags_AllowItemOverlap;
		fullSpaceWidth = false;

		guid1 = "##" + genGuid();
	}

	PropGameObject::~PropGameObject()
	{
	}

	void PropGameObject::update(bool opened)
	{
		Property::update(opened);

		std::string text = "[None]";

		if (value != nullptr)
			text = value->getName();

		ImVec2 sz = ImGui::GetContentRegionAvail();

		bool btn = false;

		if (image == nullptr)
			btn = ImGui::Button(text.c_str(), ImVec2(sz.x, 0));
		else
			btn = ImGui::ImageButtonWithText((void*)image->getHandle().idx, text.c_str(), ImVec2(16, 16), ImVec2(0, 1), ImVec2(1, 0), ImVec2(sz.x, -1.0f));
	}

	void PropGameObject::setValue(GameObject* val)
	{
		value = val;
	}
}