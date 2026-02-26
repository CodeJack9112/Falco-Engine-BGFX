#include "AudioClipEditor.h"

#include <dear-imgui/imgui.h>

#include "../Engine/Components/AudioSource.h"
#include "../Engine/Assets/AudioClip.h"
#include "../Engine/Assets/Texture.h"
#include "../Engine/Classes/Helpers.h"

namespace GX
{
	AudioClipEditor::AudioClipEditor()
	{
		setEditorName("AudioClipEditor");
	}

	AudioClipEditor::~AudioClipEditor()
	{
		if (audioSource != nullptr)
			delete audioSource;
	}

	void AudioClipEditor::init(AudioClip* clip)
	{
		audioClip = clip;

		audioSource = new AudioSource();
		audioSource->setAudioClip(audioClip);
		audioSource->setIs2D(true);
		audioSource->setOverrideTimeScale(true);
		audioSource->setOverrideTimeScaleValue(1.0f);

		playIcon = Texture::load(Helper::ExePath(), "Editor/Icons/Other/play.png", false, Texture::CompressionMethod::None, true);
		pauseIcon = Texture::load(Helper::ExePath(), "Editor/Icons/Other/pause.png", false, Texture::CompressionMethod::None, true);
		stopIcon = Texture::load(Helper::ExePath(), "Editor/Icons/Other/stop.png", false, Texture::CompressionMethod::None, true);

		setPreviewFunction([=]() { onUpdatePreview(); });
	}

	void AudioClipEditor::update()
	{
		PropertyEditor::update();
	}

	void AudioClipEditor::updateEditor()
	{
	}

	void AudioClipEditor::onUpdatePreview()
	{
		ImGui::Dummy(ImVec2(10.0f, 10.0f));

		ImGui::Text(audioClip->getName().c_str());

		ImGui::Dummy(ImVec2(10.0f, 10.0f));

		if (!audioSource->getIsPlaying())
		{
			if (ImGui::ImageButton((void*)playIcon->getHandle().idx, ImVec2(16, 16), ImVec2(0, 1), ImVec2(1, 0)))
			{
				audioSource->play();
			}
		}
		else
		{
			if (!audioSource->getIsPaused())
			{
				if (ImGui::ImageButton((void*)pauseIcon->getHandle().idx, ImVec2(16, 16), ImVec2(0, 1), ImVec2(1, 0)))
				{
					audioSource->pause();
				}
			}
			else
			{
				if (ImGui::ImageButton((void*)playIcon->getHandle().idx, ImVec2(16, 16), ImVec2(0, 1), ImVec2(1, 0)))
				{
					audioSource->resume();
				}
			}
		}

		ImGui::SameLine();

		if (ImGui::ImageButton((void*)stopIcon->getHandle().idx, ImVec2(16, 16), ImVec2(0, 1), ImVec2(1, 0)))
		{
			audioSource->stop();
		}

		ImGui::SameLine();

		bool loop = audioSource->getLooped();
		if (ImGui::Checkbox("Loop", &loop))
		{
			audioSource->setLoop(loop);
		}

		ImGui::Dummy(ImVec2(10.0f, 10.0f));

		int total = audioSource->getTotalLength();
		int pos = audioSource->getPlaybackPosition();

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
		if (ImGui::SliderInt("##sound_position", &spos, 0, 100, ""))
		{
			int seekPos = (float)spos / 100.0f * total;
			audioSource->setPlaybackPosition(seekPos);
		}
	}
}