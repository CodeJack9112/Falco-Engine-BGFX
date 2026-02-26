#include "DialogAnimationCrop.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <dear-imgui/misc/cpp/imgui_stdlib.h>

#include "../Windows/MainWindow.h"

#include "../Engine/Assets/AnimationClip.h"
#include "../Engine/Classes/IO.h"

namespace GX
{
	void DialogAnimationCrop::show(AnimationClip* value, std::function<void()> callback)
	{
		visible = true;

		animationClip = value;

		if (animationClip == nullptr)
			visible = false;
		else
		{
			completeCallback = callback;

			animationName = "New animation";
			startFrame = 0;
			endFrame = 100;

			if (!animationClip->isLoaded())
				animationClip->load();

			fileName = IO::GetFileNameWithExt(animationClip->getName());
		}
	}

	void DialogAnimationCrop::hide()
	{
		visible = false;
	}

	void DialogAnimationCrop::update()
	{
		if (!visible)
			return;

		ImGui::SetNextWindowSize(ImVec2(310.0f, 220.0f));
		bool w = ImGui::Begin("Crop animation clip", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoDocking);

		if (w)
		{
			ImGui::Text(fileName.c_str());

			///////

			ImGui::Dummy(ImVec2(5, 5));
			ImGui::Separator();
			ImGui::Dummy(ImVec2(5, 5));

			///////

			ImGui::BeginColumns("##crop_animation_clip_c_1", 2, ImGuiOldColumnFlags_NoBorder | ImGuiOldColumnFlags_NoResize);
			{
				ImGui::Text("Duration:");
			}
			ImGui::NextColumn();
			{
				ImGui::Text(std::to_string(animationClip->getDuration()).c_str());
			}
			ImGui::EndColumns();
			
			///////

			ImGui::BeginColumns("##crop_animation_clip_c_2", 2, ImGuiOldColumnFlags_NoBorder | ImGuiOldColumnFlags_NoResize);
			{
				ImGui::Text("Frames per second:");
			}
			ImGui::NextColumn();
			{
				ImGui::Text(std::to_string(animationClip->getFramesPerSecond()).c_str());
			}
			ImGui::EndColumns();

			///////

			ImGui::Dummy(ImVec2(5, 5));
			ImGui::Separator();
			ImGui::Dummy(ImVec2(5, 5));

			///////

			ImGui::BeginColumns("##crop_animation_clip_c_3", 2, ImGuiOldColumnFlags_NoBorder | ImGuiOldColumnFlags_NoResize);
			{
				ImGui::Text("Name");
			}
			ImGui::NextColumn();
			{
				ImGui::SetNextItemWidth(-1);
				ImGui::InputText("##animName", &animationName);
			}
			ImGui::EndColumns();

			///////

			ImGui::BeginColumns("##crop_animation_clip_c_3", 2, ImGuiOldColumnFlags_NoBorder | ImGuiOldColumnFlags_NoResize);
			{
				ImGui::Text("Start frame");
			}
			ImGui::NextColumn();
			{
				ImGui::SetNextItemWidth(-1);
				ImGui::InputInt("##animStartFrame", &startFrame);
			}
			ImGui::EndColumns();

			///////

			ImGui::BeginColumns("##crop_animation_clip_c_3", 2, ImGuiOldColumnFlags_NoBorder | ImGuiOldColumnFlags_NoResize);
			{
				ImGui::Text("End frame");
			}
			ImGui::NextColumn();
			{
				ImGui::SetNextItemWidth(-1);
				ImGui::InputInt("##animEndFrame", &endFrame);
			}
			ImGui::EndColumns();

			///////

			ImGui::Dummy(ImVec2(5, 5));
			ImGui::Separator();
			ImGui::Dummy(ImVec2(5, 5));

			///////

			bool closed = false;
			bool ok = false;

			if (ImGui::Button("Cancel"))
			{
				closed = true;
				ok = false;
				visible = false;
			}

			ImGui::SameLine();

			if (ImGui::Button("OK"))
			{
				closed = true;
				ok = true;
				visible = false;

				std::string animName = IO::GetFilePath(animationClip->getName()) + animationName;
				AnimationClip* clip = animationClip->crop(startFrame, endFrame, animName);
				clip->save();

				if (completeCallback != nullptr)
					completeCallback();
			}
		}

		ImGui::End();
	}
}