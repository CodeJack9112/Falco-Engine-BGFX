#include "PropAsset.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <dear-imgui/misc/cpp/imgui_stdlib.h>

#include "../Engine/UI/ImGUIWidgets.h"
#include "../Engine/Classes/IO.h"
#include "../Engine/Core/Engine.h"

#include "../Engine/Assets/Asset.h"
#include "../Engine/Assets/Texture.h"
#include "../Engine/Assets/AudioClip.h"
#include "../Engine/Assets/VideoClip.h"
#include "../Engine/Assets/Material.h"
#include "../Engine/Assets/Prefab.h"
#include "../Engine/Assets/Shader.h"
#include "../Engine/Assets/AnimationClip.h"
#include "../Engine/Assets/Cubemap.h"
#include "../Engine/Assets/Font.h"

#include "../Windows/MainWindow.h"

namespace GX
{
	PropAsset::PropAsset(PropertyEditor* ed, std::string name, Asset* val) : Property(ed, name)
	{
		setValue(val);

		flags = ImGuiTreeNodeFlags_AllowItemOverlap;
		fullSpaceWidth = false;

		guid1 = "##" + genGuid();
	}

	PropAsset::~PropAsset()
	{
	}

	void PropAsset::update(bool opened)
	{
		Property::update(opened);

		std::string fn = fileName;
		if (fn.empty())
			fn = "[None]";

		ImVec2 sz = ImGui::GetContentRegionAvail();

		bool btn = false;

		if (customImage != nullptr)
			btn = ImGui::ImageButtonWithText((void*)customImage->getHandle().idx, fn.c_str(), ImVec2(16, 16), ImVec2(0, 1), ImVec2(1, 0), ImVec2(sz.x, -1.0f));
		else if (image != nullptr)
			btn = ImGui::ImageButtonWithText((void*)image->getHandle().idx, fn.c_str(), ImVec2(16, 16), ImVec2(0, 1), ImVec2(1, 0), ImVec2(sz.x, -1.0f));
		else
			btn = ImGui::Button(fn.c_str(), ImVec2(sz.x, 0));

		if (btn)
		{
			if (onClickCallback != nullptr)
			{
				onClickCallback(this);
			}
		}
	}

	void PropAsset::setValue(Asset* val)
	{
		value = val;
		if (val != nullptr)
		{
			fileName = IO::GetFileNameWithExt(value->getName());

			if (val->getAssetType() == Texture::ASSET_TYPE)
			{
				Texture* tex = (Texture*)val;

				std::string loc = IO::GetFilePath(tex->getName());
				std::string name = IO::GetFileName(tex->getName()) + ".png";
				std::string iconCachePath = Engine::getSingleton()->getRootPath() + "Temp/IconCache/" + loc;

				image = Texture::load(iconCachePath, name, false, Texture::CompressionMethod::None, false, false);
			}
			else if (val->getAssetType() == AudioClip::ASSET_TYPE)
			{
				image = MainWindow::loadEditorIcon("Assets/audio_clip.png");
			}
			else if (val->getAssetType() == VideoClip::ASSET_TYPE)
			{
				image = MainWindow::loadEditorIcon("Assets/video_clip.png");
			}
			else if (val->getAssetType() == Material::ASSET_TYPE)
			{
				image = MainWindow::loadEditorIcon("Assets/material.png");
			}
			else if (val->getAssetType() == Prefab::ASSET_TYPE)
			{
				image = MainWindow::loadEditorIcon("Assets/prefab.png");
			}
			else if (val->getAssetType() == Shader::ASSET_TYPE)
			{
				image = MainWindow::loadEditorIcon("Assets/shader.png");
			}
			else if (val->getAssetType() == AnimationClip::ASSET_TYPE)
			{
				image = MainWindow::loadEditorIcon("Assets/animation_clip.png");
			}
			else if (val->getAssetType() == Cubemap::ASSET_TYPE)
			{
				image = MainWindow::loadEditorIcon("Assets/cubemap.png");
			}
			else if (val->getAssetType() == Font::ASSET_TYPE)
			{
				image = MainWindow::loadEditorIcon("Assets/font.png");
			}
		}
		else
		{
			image = nullptr;
			fileName = "";
		}
	}
}