#pragma once

#include "../Engine/glm/vec2.hpp"

#include "PropertyEditor.h"

namespace GX
{
	class AudioClip;
	class AudioSource;
	class Texture;

	class AudioClipEditor : public PropertyEditor
	{
	private:
		AudioClip* audioClip = nullptr;
		AudioSource* audioSource = nullptr;

		Texture* playIcon = nullptr;
		Texture* pauseIcon = nullptr;
		Texture* stopIcon = nullptr;

	public:
		AudioClipEditor();
		~AudioClipEditor();

		virtual void init(AudioClip* clip);
		virtual void update();
		virtual void updateEditor();

	private:
		//Preview
		static glm::vec2 prevSize;

		void onUpdatePreview();
	};
}