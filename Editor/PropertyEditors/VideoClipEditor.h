#pragma once

#include "../Engine/glm/vec2.hpp"

#include "PropertyEditor.h"

namespace GX
{
	class VideoClip;
	class VideoPlayer;
	class Texture;

	class VideoClipEditor : public PropertyEditor
	{
	private:
		VideoClip* videoClip = nullptr;
		VideoPlayer* videoPlayer = nullptr;

		Texture* playIcon = nullptr;
		Texture* pauseIcon = nullptr;
		Texture* stopIcon = nullptr;

	public:
		VideoClipEditor();
		~VideoClipEditor();

		virtual void init(VideoClip* clip);
		virtual void update();
		virtual void updateEditor();

	private:
		//Preview
		static glm::vec2 prevSize;

		void onUpdatePreview();
	};
}