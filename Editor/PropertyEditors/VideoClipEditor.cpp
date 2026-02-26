#include "VideoClipEditor.h"

#include <dear-imgui/imgui.h>

#include "../Engine/Assets/VideoClip.h"
#include "../Engine/Assets/Texture.h"
#include "../Engine/Classes/Helpers.h"
#include "../Engine/Components/VideoPlayer.h"
#include "../Engine/Renderer/Renderer.h"
#include "../Engine/Core/Time.h"

namespace GX
{
	VideoClipEditor::VideoClipEditor()
	{
		setEditorName("VideoClipEditor");
	}

	VideoClipEditor::~VideoClipEditor()
	{
		if (videoPlayer != nullptr)
			delete videoPlayer;

		videoPlayer = nullptr;
		videoClip = nullptr;
	}

	void VideoClipEditor::init(VideoClip* clip)
	{
		videoClip = clip;

		videoPlayer = new VideoPlayer();
		videoPlayer->setVideoClip(videoClip);

		playIcon = Texture::load(Helper::ExePath(), "Editor/Icons/Other/play.png", false, Texture::CompressionMethod::None, true);
		pauseIcon = Texture::load(Helper::ExePath(), "Editor/Icons/Other/pause.png", false, Texture::CompressionMethod::None, true);
		stopIcon = Texture::load(Helper::ExePath(), "Editor/Icons/Other/stop.png", false, Texture::CompressionMethod::None, true);

		setPreviewFunction([=]() { onUpdatePreview(); });
	}

	void VideoClipEditor::update()
	{
		PropertyEditor::update();
	}

	void VideoClipEditor::updateEditor()
	{
	}

	void VideoClipEditor::onUpdatePreview()
	{
		ImGui::Dummy(ImVec2(10.0f, 10.0f));

		ImGui::Text(videoClip->getName().c_str());

		ImGui::Dummy(ImVec2(10.0f, 10.0f));

		if (!videoPlayer->getIsPlaying())
		{
			if (ImGui::ImageButton((void*)playIcon->getHandle().idx, ImVec2(16, 16), ImVec2(0, 1), ImVec2(1, 0)))
			{
				videoPlayer->play();
			}
		}
		else
		{
			if (!videoPlayer->getIsPaused())
			{
				if (ImGui::ImageButton((void*)pauseIcon->getHandle().idx, ImVec2(16, 16), ImVec2(0, 1), ImVec2(1, 0)))
				{
					videoPlayer->pause();
				}
			}
			else
			{
				if (ImGui::ImageButton((void*)playIcon->getHandle().idx, ImVec2(16, 16), ImVec2(0, 1), ImVec2(1, 0)))
				{
					videoPlayer->resume();
				}
			}
		}

		ImGui::SameLine();

		if (ImGui::ImageButton((void*)stopIcon->getHandle().idx, ImVec2(16, 16), ImVec2(0, 1), ImVec2(1, 0)))
		{
			videoPlayer->stop();
		}

		ImGui::SameLine();

		bool loop = videoPlayer->getLoop();
		if (ImGui::Checkbox("Loop", &loop))
		{
			videoPlayer->setLoop(loop);
		}

		ImGui::Dummy(ImVec2(10.0f, 10.0f));

		int total = videoPlayer->getTotalLength();
		int pos = videoPlayer->getPlaybackPosition();

		int spos = 0;
		int mins = 0;
		int secs = 0;

		std::string totalTime = "00:00";
		std::string tMins = "";
		std::string tSecs = "";

		if (total > 0)
		{
			spos = (float)pos / (float)total * 100;
			mins = pos / 60;
			secs = pos - (mins * 60);

			tMins = std::to_string(total / 60);
			tSecs = std::to_string(total - (total / 60 * 60));

			if (tMins.length() == 1) tMins = '0' + tMins;
			if (tSecs.length() == 1) tSecs = '0' + tSecs;

			totalTime = tMins + ":" + tSecs;
		}

		tMins = std::to_string(mins);
		tSecs = std::to_string(secs);

		if (tMins.length() == 1) tMins = '0' + tMins;
		if (tSecs.length() == 1) tSecs = '0' + tSecs;

		std::string text = tMins + ":" + tSecs + " / " + totalTime;

		ImGui::Text(text.c_str());
		if (ImGui::SliderInt("##video_position", &spos, 0, 100, ""))
		{
			int seekPos = (float)spos / 100.0f * total;
			videoPlayer->setPlaybackPosition(seekPos);
		}

		ImGui::Dummy(ImVec2(10, 10));

		int y = ImGui::GetCursorPosY();

		Texture* frameTex = videoPlayer->getTexture();

		if (frameTex != nullptr)
		{
			ImVec2 sz = ImGui::GetContentRegionAvail();
			sz.x -= 1;
			float x = sz.x;

			float w = frameTex->getOriginalWidth();
			float h = frameTex->getOriginalHeight();
			float aspect = h / w;
			float aspectW = w / h;

			sz.y = sz.x * aspect;

			if (previewHeight - y < sz.y)
			{
				sz.y = previewHeight - y - 5;
				sz.x = sz.y * aspectW;
			}

			if (sz.y < 0) sz.y = 0;
			if (sz.x < 0) sz.x = 0;

			ImGui::SetCursorPosX(x / 2.0f - sz.x / 2.0f);

			ImGui::Image((void*)frameTex->getHandle().idx, sz, ImVec2(0, 1), ImVec2(1, 0));
		}

		videoPlayer->onUpdate(Time::getDeltaTime());
	}
}